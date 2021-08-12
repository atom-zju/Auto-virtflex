#include "pnode.h"
#include <iostream>
#include <cassert>
#include "topo_change_d.h"

pnode::pnode(int id):node(id), owner_vnode(NULL){
}

pnode::~pnode(){
	for(auto& x: bw_rd_sq_grabber)
		if(x) delete x;
	for(auto& x: bw_wr_sq_grabber)
		if(x) delete x;
	for(auto& x: vnode_map){
		delete x.second;
	}
}

void pnode::register_vnode(int vm_id, vnode* n){
	if(vnode_map.find(vm_id) == vnode_map.end()){
		vnode_map[vm_id] = n;
	}
}
void pnode::unregister_vnode(int vm_id, vnode* n){
	if(vnode_map.find(vm_id) != vnode_map.end()){
		assert(vnode_map[vm_id] == n);
		vnode_map.erase(vm_id);
	}
}

void pnode::update_vnode_map(int ts){
	cout << UNIX_TS << "\tupdating pnode, id: " << pnode_id << endl;
	if(file_output)
	of << UNIX_TS << "\tupdating pnode, id: " << pnode_id << endl;
	active_vnodes = 0;
	total_vnodes = 0;
	recent_cpu_usage = 0;
	for(auto& x: vnode_map){
		if(x.second->ts < ts){
			if(x.second == owner_vnode)
				owner_vnode = NULL;
			vnode_map.erase(x.first);
		}
		else{
			total_vnodes++;
			recent_cpu_usage += x.second->get_recent_vcpu_usage();
			if(x.second->enabled){
				active_vnodes++;
			}
			else{
				if(x.second == owner_vnode)
					owner_vnode = NULL;
			}
		}
	}
	//cout << UNIX_TS<< "\ttotal_vnodes: " << total_vnodes << endl;
	//cout << UNIX_TS<< "\tactive_vnodes: " << active_vnodes << endl;
	
	if(owner_vnode == NULL){
		pick_owner_vnode();
	}
	if(owner_vnode)
		get_owner_vnode_stat();
		
}

void pnode::pick_owner_vnode(){
	// right now just randomly pick an enabled node
	assert(!owner_vnode);
	for(auto& x: vnode_map){
		if(x.first != 0 && x.second->enabled){
			owner_vnode = x.second;
			return;
		}
	}
}

void pnode::get_owner_vnode_stat(){
	// right now the only stat to get is bw usage
	assert(owner_vnode);
	//owner_vnode->clear_bw_sq();
	//owner_vnode->read_bw_usage_from_xs();
	//copy_owner_vnode_bw_usage();
	grab_bw_sample();
	scatter_bw_sample();
}

static string generate_bw_sample_xs_dir(vnode* vn, string rd_or_wr, int ch){
	assert(rd_or_wr == "rd" || rd_or_wr == "wr");
	string res = vn->xs_path;
	res += "/bw_usage_"+ to_string(ch)+"_"+rd_or_wr;
	return res;
}
static bool bw_sample_xs_dir_is_valid(struct xs_handle* xs, string dir){
	string tmp;
	return read_from_xenstore_path(xs, dir+"/curr_sample_num", tmp) == 0;
}

void pnode::grab_bw_sample(){
	assert(owner_vnode);
	owner_vnode->change_pnode_owner_xs(true);
	auto xs_handle = owner_vnode->topod->xs;
	int ch = 0;
	do{
		// gether bw rd sample
		string xs_dir = generate_bw_sample_xs_dir(owner_vnode, "rd", ch);
		if(bw_sample_xs_dir_is_valid(xs_handle, xs_dir)){
			if(ch >= bw_rd_sq_grabber.size())
				bw_rd_sq_grabber.push_back(new sample_queue<int> ("", xs_handle,
								dir(SYS_VM_ID, pnode_id)));
			bw_rd_sq_grabber[ch]->clear_sample();
			bw_rd_sq_grabber[ch]->xs_dir = xs_dir;
			bw_rd_sq_grabber[ch]->name = BW_USAGE_SQ;
			if( bw_rd_sq_grabber[ch]->get_sample(owner_vnode->owner_vm->start_time_sec_unix) < 0)
				cerr << "can't get sample in grab_bw_sample" <<endl;	
		}
		else
			break;
		
		// gather bw wr sample
		xs_dir = generate_bw_sample_xs_dir(owner_vnode, "wr", ch);
		if(bw_sample_xs_dir_is_valid(xs_handle, xs_dir)){
                        if(ch >= bw_wr_sq_grabber.size())
                                bw_wr_sq_grabber.push_back(new sample_queue<int> ("", xs_handle, 
								dir(SYS_VM_ID, pnode_id)));
                        bw_wr_sq_grabber[ch]->clear_sample();
                        bw_wr_sq_grabber[ch]->xs_dir = xs_dir;
			bw_rd_sq_grabber[ch]->name = BW_USAGE_SQ;
                        if( bw_wr_sq_grabber[ch]->get_sample(owner_vnode->owner_vm->start_time_sec_unix) < 0)
                                cerr << "can't get sample in grab_bw_sample" <<endl;
                }
                else
                        cerr<< "different number of rd and wr chns in grab_bw_sample" <<endl;
		
		ch++;
		
	}while(1);
	
	// put bw samples from grabber to pnode sq
	for(int i = 0; i < bw_rd_sq_grabber.size(); i++ ){
		if(i >= bw_rd_channel_sample.size())
			bw_rd_channel_sample.push_back(new sample_queue<int> ("", NULL,
							dir(SYS_VM_ID, pnode_id), BW_USAGE_SQ));
                bw_rd_channel_sample[i]->merge_sample_queue(bw_rd_sq_grabber[i]);
	}
	for(int i = 0; i < bw_wr_sq_grabber.size(); i++ ){
                if(i >= bw_wr_channel_sample.size())
                        bw_wr_channel_sample.push_back(new sample_queue<int> ("", NULL,
							dir(SYS_VM_ID, pnode_id), BW_USAGE_SQ));
                bw_wr_channel_sample[i]->merge_sample_queue(bw_wr_sq_grabber[i]);
        }
	
	// below is for test:
	//cout  << "bw_rd_sq_grabber : " << endl;
	//for(auto& x: bw_rd_sq_grabber){
	//	cout << "\t sq: ";
	//	for (auto& y: x->sample)
	//		cout << y.first << ", " << y.second << " | ";
	//	cout << endl;
	//}
	//cout  << "bw_wr_sq_grabber : " << endl;
        //for(auto& x: bw_wr_sq_grabber){
        //        cout << "\t sq: ";
        //        for (auto& y: x->sample)
        //                cout << y.first << ", " << y.second << " | ";
        //        cout << endl;
        //}
}

void pnode::scatter_bw_sample(){
	unordered_map<int, float> m;
	float acc;
	// scatter rd samples
	for(int i=0; i < bw_rd_sq_grabber.size(); i++){
		for(auto& data: bw_rd_sq_grabber[i]->sample){
			// for each sample point (timestamp, bw_measure)
			//cout << "data:" << data.first << ", " << data.second; 
			m.clear();
			acc = 0;
			for(auto& vn: vnode_map){
				auto nearst_p = vn.second->cpu_usage_sq->get_nearst_data_point(data.first);
				if(nearst_p.first < 0 || nearst_p.second <0)
					continue;
				m[vn.first] = nearst_p.second;
				acc+=nearst_p.second;
			}
			//cout << " acc: "  << acc << " ||||| ";
			//cout << " Scatter bw samples: " << data.first << ", " << data.second << endl; 
			for(auto& x: m){
				int measure;
				if( acc == 0 )
					measure = 0;
				else
					measure = (int) (data.second * (x.second / acc));
				//cout << " vm:" << x.first << " cpu_load: " << x.second << ", bw: " << measure; 
				if(i >= vnode_map[x.first]->bw_rd_channel_sample.size())
					vnode_map[x.first]->bw_rd_channel_sample.push_back(
						new sample_queue<int>("", vnode_map[x.first]->topod->xs,
							dir(x.first, vnode_map[x.first]->vnode_id), BW_USAGE_SQ));
				vnode_map[x.first]->bw_rd_channel_sample[i]->
					insert_sample(make_pair(data.first, measure));
			}
			//cout << endl;		
		}
	}
	// scatter wr samples
	for(int i=0; i < bw_wr_sq_grabber.size(); i++){
		for(auto& data: bw_wr_sq_grabber[i]->sample){
			// for each sample point (timestamp, bw_measure)
			//cout << "data:" << data.first << ", " << data.second; 
			m.clear();
			acc = 0;
			for(auto& vn: vnode_map){
				auto nearst_p = vn.second->cpu_usage_sq->get_nearst_data_point(data.first);
				if(nearst_p.first < 0 || nearst_p.second <0)
					continue;
				m[vn.first] = nearst_p.second;
				acc+=nearst_p.second;
			}
			//cout << " acc: "  << acc << " ||||| ";
			for(auto& x: m){
				int measure;
				if( acc == 0 )
					measure = 0;
				else
					measure = (int) (data.second * (x.second / acc));
				//cout << " vm:" << x.first << " cpu_load: " << x.second << ", bw: " << measure; 
				if(i >= vnode_map[x.first]->bw_wr_channel_sample.size())
					vnode_map[x.first]->bw_wr_channel_sample.push_back(
						new sample_queue<int>("", vnode_map[x.first]->topod->xs,
							dir(x.first, vnode_map[x.first]->vnode_id), BW_USAGE_SQ));
				vnode_map[x.first]->bw_wr_channel_sample[i]->
					insert_sample(make_pair(data.first, measure));
			}
			//cout << endl;		
		}
	}
	// below is for test: 
	//for(auto& vn: vnode_map){
	//	cout << "vm: " << vn.first <<endl;
	//	for(auto& sq: vn.second->bw_rd_channel_sample){
	//		sq->print();
	//	}
	//}
}

//void pnode::copy_owner_vnode_bw_usage(){
//	assert(owner_vnode);
//	/* 1. create sample queue
//	   2. merge samples from vnode sample queue */
//	// merge bw rd sq
//	int last_channel = bw_rd_channel_sample.size()-1;
//	while(bw_rd_channel_sample.size() < owner_vnode->bw_rd_channel_sample.size()){
//		bw_rd_channel_sample.push_back(new sample_queue<int>(
//				owner_vnode->bw_rd_channel_sample[last_channel+1]->name,
//				NULL,
//				owner_vnode->bw_rd_channel_sample[last_channel+1]->name));	
//		last_channel++;
//	}
//	for(int i=0; i < bw_rd_channel_sample.size() && i < owner_vnode->bw_rd_channel_sample.size(); i++){
//		assert(bw_rd_channel_sample[i]);
//		bw_rd_channel_sample[i]->merge_sample_queue(
//				owner_vnode->bw_rd_channel_sample[i]);
//	}
//	// merge bw wr sq
//	last_channel = bw_wr_channel_sample.size()-1;
//	while(bw_wr_channel_sample.size() < owner_vnode->bw_wr_channel_sample.size()){
//                bw_wr_channel_sample.push_back(new sample_queue<int>(
//				owner_vnode->bw_wr_channel_sample[last_channel+1]->name,
//				NULL,
//				owner_vnode->bw_wr_channel_sample[last_channel+1]->name));
//		last_channel++;
//        }
//        for(int i=0; i < bw_wr_channel_sample.size() && i < owner_vnode->bw_wr_channel_sample.size(); i++){
//		assert(bw_wr_channel_sample[i]);
//                bw_wr_channel_sample[i]->merge_sample_queue(
//                                owner_vnode->bw_wr_channel_sample[i]);
//        }
//
//}

long pnode::average_bw_usage(){
	if(!owner_vnode)
		return -1;
	if(bw_rd_channel_sample.empty() || bw_wr_channel_sample.empty()){
		return -1;
	}
	int sample_count = 0;
	long sum = 0;
	long long valid_ts_ms = ((long long)time(0))*1000 - VALID_SAMPLE_INTERVAL_MS;
	for(auto& y: bw_rd_channel_sample){
		int avg = y->average_value_since_ts(valid_ts_ms);
		if(avg >= 0){
			sum+= avg;
			sample_count++;
		}
	}

        for(auto& y: bw_wr_channel_sample){
		int avg = y->average_value_since_ts(valid_ts_ms);
		if(avg >= 0){
			sum+= avg;
			sample_count++;
		}
        }

	if(sample_count == 0)
		return -1;
	return sum/sample_count;
}
