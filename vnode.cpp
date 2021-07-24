#include "vnode.h"
#include "topo_change_d.h"
#include <time.h>
#include <iostream>
#include <cassert>

#define get_name(var)  #var

void vnode::update_node(unsigned int ts){
	this->ts = ts;
	// update capacity and cpu list
	string tmp;
	vector<string> tmp_list;

	assert(topod);

	read_from_xenstore_path(topod->xs, string(xs_path).append("/pnode"), tmp);
	pnode_id = stoi(tmp);
	//cout << "pnode_id: " << pnode_id <<endl;

	assert(this->owner_vm);
	topod->register_pvnode(this->owner_vm->vm_id, this, pnode_id);

	read_from_xenstore_path(topod->xs, string(xs_path).append("/capacity"), tmp);
	capacity = stoi(tmp);
	//cout << "capacity: " << capacity <<endl;

	read_from_xenstore_path(topod->xs, string(xs_path).append("/target"), tmp);
	target = stoi(tmp);
	//cout << "target: " << target <<endl;
	//cout << "enabled: " << enabled <<endl;

	
	// be carefull not to insert all the vcpus one more time everytime you update node
	// right now because set doesn't allow duplicates, this problem can be prevented
	list_xenstore_directory(topod->xs, string(xs_path).append("/vcpu"),tmp_list);
	struct timeval now;
	gettimeofday(&now, NULL);
	for(auto& x: tmp_list){
		//cout<< "vcpu: " << x <<endl;
		//cpu_set.insert(cpu(stoi(x),true, owner_vm));
		if(cpu_map.find(stoi(x)) == cpu_map.end()){
			cpu_map[stoi(x)] = cpu(stoi(x),true, owner_vm);
			cpu_map[stoi(x)].init(&now);
		}
		cpu_map[stoi(x)].update_usage(&now);
	}
	if(!cpu_usage_sq){
		cpu_usage_sq = new sample_queue<float>(string(""), NULL,
				dir(owner_vm->vm_id, vnode_id), CPU_USAGE_SQ, MAX_SAMPLE_SIZE, (node*)this);
	}
	cpu_usage_sq->get_sample();
	//cpu_usage_sq->calculate_averages();
	//cout << UNIX_TS << "node: " << vnode_id << " cpu sq short avg: " << cpu_usage_sq->get_short_average();
	//cout << " cpu sq size : " << cpu_usage_sq->sample.size() << endl;

	// calculate low target base on capacity, formula:
	// low_target = (80*1024 + capacity/1024*17)
	if(vnode_id == 0){
		low_target = (80*1024 + capacity/1024*17)+200*1024;
		low_target += (capacity-low_target)/cpu_map.size();
	}
	else{
		low_target = (80*1024 + capacity/1024*17);
	}
	//cout << "low_target: " << low_target << endl;

	
}

int vnode::shrink(){
	// 1. set the topo_change flag to be 2, path: /numa/topo_change
	// 2. change target to low_target 
	// 3. disable all vcpus
	enabled = false;
	change_pnode_owner_xs(false);
	string topo_change_flag(xs_path.substr(0, xs_path.find_last_of("/\\")));
	topo_change_flag = topo_change_flag.substr(0, topo_change_flag.find_last_of("/\\"));
	topo_change_flag.append("/topo_change");
	//cout << "topo_change_flag path: " << topo_change_flag << endl;
	
	assert(topod);
	for(auto& x: cpu_map){
		if(x.first != 0)
			x.second.disable();
	}
	write_to_xenstore_path(topod->xs, topo_change_flag,string("2"));
	write_to_xenstore_path(topod->xs, string(xs_path).append("/target"),to_string(low_target));
	
	return 0;	
}

int vnode::expand(){
	// 1. set the topo_change flag to be 1, path: /numa/topo_change
	// 2. change target to capacity
	// 3. enable all vcpus
	for(auto& x: cpu_map){
		if(x.first != 0)
			x.second.enable();
	}
	enabled = true;
	string topo_change_flag(xs_path.substr(0, xs_path.find_last_of("/\\")));
	topo_change_flag = topo_change_flag.substr(0, topo_change_flag.find_last_of("/\\"));
	topo_change_flag.append("/topo_change");
	//cout << "topo_change_flag path: " << topo_change_flag << endl;
	assert(topod);
	write_to_xenstore_path(topod->xs, string(xs_path).append("/target"),to_string(capacity));
	write_to_xenstore_path(topod->xs, topo_change_flag,string("1"));
	return 0;
}

long vnode::average_bw_usage(){
	if(!enabled)
		return -1;
	long long usg = get_recent_vcpu_usage();
	long long total_usg = topod->pnode_cpu_usage(pnode_id);
	assert(usg <= total_usg && usg >= 0);
	if(total_usg == 0){
		return topod->pnode_average_bw_usage(pnode_id);
	}
	else
		return (long)((long long)topod->pnode_average_bw_usage(pnode_id)*usg)/total_usg;
}

void vnode::calculate_bw_sample_queue_averages(){
	for(int i=0; i < bw_rd_channel_sample.size(); i++){
		bw_rd_channel_sample[i]->calculate_averages();
	}
	for(int i=0; i < bw_wr_channel_sample.size(); i++){
                bw_wr_channel_sample[i]->calculate_averages();
        }
}

long long vnode::bw_short_average(){
	long long avg = 0;
	int cnt = 0;
	//assert(bw_rd_channel_sample.size() &&
	//	bw_wr_channel_sample.size());
	for(int i=0; i < bw_rd_channel_sample.size(); i++){
                avg += bw_rd_channel_sample[i]->get_short_average();
		cnt++;
        }
	for(int i=0; i < bw_wr_channel_sample.size(); i++){
                avg += bw_wr_channel_sample[i]->get_short_average();
		cnt++;
        }
	if(cnt == 0 ){
		cout << UNIX_TS<< "bw_short_average is 0 for vnode " << vnode_id << endl;
		if(file_output)
		of << UNIX_TS<< "bw_short_average is 0 for vnode " << vnode_id << endl;
	}
	return cnt?avg/cnt:0;
}
long long vnode::bw_long_average(){
	long long avg = 0;
	int cnt = 0;
	//assert(bw_rd_channel_sample.size() &&
	//	bw_wr_channel_sample.size());
	for(int i=0; i < bw_rd_channel_sample.size(); i++){
                avg += bw_rd_channel_sample[i]->get_long_average();
		cnt++;
        }
	for(int i=0; i < bw_wr_channel_sample.size(); i++){
                avg += bw_wr_channel_sample[i]->get_long_average();
		cnt++;
        }
	if(cnt == 0 ){
		cout << UNIX_TS<< "bw_long_average is 0 for vnode " << vnode_id << endl;
		if(file_output)
		of << UNIX_TS<< "bw_long_average is 0 for vnode " << vnode_id << endl;
	}
	return cnt?avg/cnt:0;
}

int vnode::get_topo_changeness(){
	// calculate short-term average
	// calcualte long-term average
	calculate_bw_sample_queue_averages();
	long long short_avg = bw_short_average();
	cout << UNIX_TS<< "vnode::get_topo_changeness(), bw_short_average: " << short_avg<< endl;
	if(file_output)
	of << UNIX_TS<< "vnode::get_topo_changeness(), bw_short_average: " << short_avg<< endl;
	long long long_avg = bw_long_average();
	cout << UNIX_TS<< "vnode::get_topo_changeness(), bw_long_average: " << long_avg<< endl;
	if(file_output)	
	of << UNIX_TS<< "vnode::get_topo_changeness(), bw_long_average: " << long_avg<< endl;
	// TODO need some kind of normalization to regulate the results
	return (short_avg - long_avg)/50; // divided by 50 to normalized the reauslt
}

int vnode::active_nodes_in_pnode(){
	assert(topod->pnode_list.size()>=pnode_id+1);
	return topod->pnode_list[pnode_id]->active_vnodes;
}

long long vnode::get_recent_vcpu_usage(){
	long long usg = 0;
	for(auto& x: cpu_map){
		usg+= x.second.recent_usage_ms;
	}
	return usg;
}

float vnode::get_average_vcpu_load(){
	float res = 0;
	for(auto& x: cpu_map){
		res += x.second.recent_usage_percent;
	}
	return res/(float)cpu_map.size();
}

int vnode::change_pnode_owner_xs(bool own){
	if (write_to_xenstore_path(topod->xs, xs_path+"/pnode_owner", own? string("1"): string("0")) != 0){
		cout<< UNIX_TS << "Error when writing to pnode_owner" << endl;
		if(file_output)
		of<< UNIX_TS << "Error when writing to pnode_owner" << endl;
		return -1;
	}
	return 0;
}


//void vnode::read_bw_usage_from_xs(){
//	string tmp;
//	change_pnode_owner_xs(true);
//	/* get rd bw usage info */
//	cout << UNIX_TS<< "Read_bw_usage_from_xs: pnode: "  << pnode_id << " vnode: " << vnode_id <<endl;
//	if(file_output)
//	of << UNIX_TS<< "Read_bw_usage_from_xs: pnode: "  << pnode_id << " vnode: " << vnode_id <<endl;
//	int num_chn_rd = 0;
//	do{
//		string chn_name("/bw_usage_");
//		chn_name.append(to_string(num_chn_rd)).append("_rd");
//		
//		if(read_from_xenstore_path(topod->xs, string(xs_path)+chn_name+"/curr_sample_num", tmp) == 0){
//			if(bw_rd_channel_sample.size() == num_chn_rd){
//				bw_rd_channel_sample.push_back(new sample_queue<int>(
//						string(xs_path)+chn_name,
//						topod->xs,
//						chn_name));
//			}
//			assert(bw_rd_channel_sample[num_chn_rd]);
//			if(bw_rd_channel_sample[num_chn_rd]->
//					get_sample(((long long)(owner_vm->start_time_sec_unix))))
//				cerr << "Error getting sample from xs for " << chn_name << endl;
//		}
//		else{
//			break;
//		}
//		num_chn_rd++;
//	}while(1);	
//		
//	/* get wr bw usage info */
//	int num_chn_wr = 0;
//	do{
//		string chn_name("/bw_usage_");
//		chn_name.append(to_string(num_chn_wr)).append("_wr");
//		
//		if(read_from_xenstore_path(topod->xs, string(xs_path)+chn_name+"/curr_sample_num", tmp) == 0){
//			if(bw_wr_channel_sample.size() == num_chn_wr){
//				bw_wr_channel_sample.push_back(new sample_queue<int>(
//						string(xs_path)+chn_name,
//						topod->xs,
//						chn_name));
//                        }
//			assert(bw_wr_channel_sample[num_chn_wr]);
//			if(bw_wr_channel_sample[num_chn_wr]->
//					get_sample(((long long)(owner_vm->start_time_sec_unix))))
//				cerr << "Error getting sample from xs for " << chn_name << endl;
//		}
//		else{
//			break;
//		}
//		num_chn_wr++;
//	}while(1);	
//	
//	assert(num_chn_rd == num_chn_wr);
//	
//}

void vnode::clear_bw_sq(){
	for(auto&& sq: bw_rd_channel_sample){
		sq->clear_sample();
	}
	for(auto&& sq: bw_wr_channel_sample){
		sq->clear_sample();
	}
}
