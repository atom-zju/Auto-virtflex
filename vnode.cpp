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
		return -1;\
	long long usg = get_recent_vcpu_usage();
	long long total_usg = topod->pnode_cpu_usage(pnode_id);
	assert(usg <= total_usg && usg >= 0);
	if(total_usg == 0){
		return topod->pnode_average_bw_usage(pnode_id);
	}
	else
		return (long)((long long)topod->pnode_average_bw_usage(pnode_id)*usg)/total_usg;
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
		cout << "Error when writing to pnode_owner" << endl;
		return -1;
	}
	return 0;
}

/*
        xenstore stats directory structure:
                numa/node/<node_id>/bw_usage_rd_0/curr_sample_num
                                                 /sample0
                                                 /sample0_ts
                                                 /sample1
                                                 /sample1_ts
                                                 ......
*/

/*
	deque<pair<long long, int>>& samples are sorted in chronological order with the earliest sample in front
	This return_insert_index returns the idx the sample need to be inserted, and returns -1 if already have
	samples of the same timestamps
*/

static int return_insert_index(deque<pair<long long, int>>& samples, int lo, int hi, long long timestamp){
	if(samples.empty())
		return 0;
	if(lo == hi){
		if(samples[lo].first == timestamp)
			return -1;
		else if( timestamp < samples[lo].first)
			return lo;
		else
			return lo+1;
	}
	int mid = (hi - lo)/2 + lo;
	if (samples[mid].first == timestamp){
			return -1;
	}
	else if (timestamp < samples[mid].first){
		if(mid == lo)
			return return_insert_index(samples, lo, mid, timestamp);
		else
			return return_insert_index(samples, lo, mid-1, timestamp);
	}
	else{
		if(mid == hi)
			return return_insert_index(samples, mid, hi, timestamp);
		else
			return return_insert_index(samples, mid+1, hi, timestamp);
	}
}



static void crawl_bw_samples_from_xs(struct xs_handle * xs, string dir, deque<pair<long long, int>>& samples, int max_sample_size, long long valid_ts){
	string val_str;
	string timestamp_str;
	int sample_cnt = 0;
	do{
		if(read_from_xenstore_path(xs, dir+"/sample"+to_string(sample_cnt), val_str) == 0){
			if(read_from_xenstore_path(xs, dir+"/sample"+to_string(sample_cnt)+"_ts", timestamp_str) == 0){
				long long ts;
				int val;
				try{
					ts = stoll(timestamp_str);
					val = stoi(val_str);
				}
				catch(...){
					cerr << "stoll or stoi error in crawl_bw_samples_from_xs" << endl;
					sample_cnt++;
					continue;
				}
				if(ts < valid_ts){
					sample_cnt++;
					continue;
				}
				int idx =  return_insert_index(samples, 0, samples.size()-1, ts);
				if(idx >= 0)
					samples.insert(samples.begin()+ idx, make_pair(ts, val));
				if(samples.size() > max_sample_size)
					samples.pop_front();
			}
		}
		else{
			break;
		}
		sample_cnt++;
	} while(1);

}


void vnode::read_bw_usage_from_xs(){
	string tmp;
	change_pnode_owner_xs(true);
	/* get rd bw usage info */
	cout << "Read_bw_usage_from_xs: pnode: "  << pnode_id << " vnode: " << vnode_id <<endl;
	int num_chn_rd = 0;
	long long valid_ts_ms = (time(0) - owner_vm->start_time)*1000 - VALID_SAMPLE_INTERVAL_MS;
	do{
		string chn_name("/bw_usage_");
		chn_name.append(to_string(num_chn_rd)).append("_rd");
		
		if(read_from_xenstore_path(topod->xs, string(xs_path)+chn_name+"/curr_sample_num", tmp) == 0){
			if(bw_rd_channel_sample.size() == num_chn_rd){
				bw_rd_channel_sample.resize(num_chn_rd+1);
			}
			crawl_bw_samples_from_xs(topod->xs, string(xs_path)+chn_name, bw_rd_channel_sample[num_chn_rd], max_sample_size, valid_ts_ms);
		}
		else{
			break;
		}
		num_chn_rd++;
	}while(1);	
		
	/* get wr bw usage info */
	int num_chn_wr = 0;
	do{
		string chn_name("/bw_usage_");
		chn_name.append(to_string(num_chn_wr)).append("_wr");
		
		if(read_from_xenstore_path(topod->xs, string(xs_path)+chn_name+"/curr_sample_num", tmp) == 0){
			if(bw_wr_channel_sample.size() == num_chn_wr){
                                bw_wr_channel_sample.resize(num_chn_wr+1);
                        }
                        crawl_bw_samples_from_xs(topod->xs, string(xs_path)+chn_name, bw_wr_channel_sample[num_chn_wr], max_sample_size, valid_ts_ms);
		}
		else{
			break;
		}
		num_chn_wr++;
	}while(1);	
	
	assert(num_chn_rd == num_chn_wr);
	
}
