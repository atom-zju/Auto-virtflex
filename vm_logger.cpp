#include "vm_logger.h"
#include "topo_change_d.h"
#include <cassert>

using namespace std;

vm_logger::~vm_logger(){
	flush_log_to_disk(true);
	log_file.close();
}

void vm_logger::init(){
	assert(owner_vm);
	log_file.open(file_path);
	log_file << "Header: <vm id>, <num_nodes>, <sample_interval/microseconds>" << endl;
	log_file << "\t" << owner_vm->vm_id << ", " << owner_vm->total_node << 
				", " << owner_vm->topod->interval_us << endl;
	log_file << "Content: ts, vCPU usage_node_0, bw_usage_node_0, vCPU usage_node_1, bw_usage_node_1 .... " << endl;
}

//void vm_logger::log(){
//	assert(owner_vm);
//	assert(owner_vm->total_node > 0 );
//	num_nodes =  owner_vm->total_node;
//	log_file << owner_vm->ts;
//	for(int i=0; i < num_nodes; i++){
//		auto node = owner_vm->get_vnode_by_id(i);
//		if(!node)
//			log_file << ", -1, -1";
//		else{
//			log_file << ", " << node->get_recent_vcpu_usage() << ", " << node->average_bw_usage();
//		}
//	}
//	log_file << endl;
//}

int vm_logger::floor_idx_of_in_mem_entries(long long unix_ts_ms){
	int lo = 0, hi = recent_entries.size()-1;
	if(recent_entries.empty() || unix_ts_ms < recent_entries[lo].first)
		return -1;
	if(unix_ts_ms > recent_entries[hi].first)
		return hi;
	while(lo <= hi){
		int mid = (lo + hi)/2;
		if(recent_entries[mid].first == unix_ts_ms){
			return mid;
		}
		if(unix_ts_ms < recent_entries[mid].first){
			hi = mid - 1;
		}
		else
			lo = mid + 1;
	}
	return hi;
}

void vm_logger::insert_log_entry(long long unix_ts_ms, string log_entry){
	long long valid_unix_ts = time(0)*1000 - time_window_in_sec*1000;
	if(unix_ts_ms < valid_unix_ts)
		return;
	if(recent_entries.empty()){
		recent_entries.push_back({unix_ts_ms, log_entry});
	}
	else{
		int idx = floor_idx_of_in_mem_entries(unix_ts_ms);
		recent_entries.insert(recent_entries.begin()+idx, {unix_ts_ms, log_entry});
	}
}

void vm_logger::flush_log_to_disk(bool flush_all){
	long long valid_unix_ts;
	if(flush_all)
       		valid_unix_ts = time(0)*1000;
	else
       		valid_unix_ts = time(0)*1000 - time_window_in_sec*1000;
	int idx = floor_idx_of_in_mem_entries(valid_unix_ts);
	for(int i=0; i< idx; i++){
		log_file << recent_entries[0].first << ", " << recent_entries[0].second << endl;
		recent_entries.erase(recent_entries.begin());
	}
}
