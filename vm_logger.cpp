#include "vm_logger.h"
#include "topo_change_d.h"
#include "util.h"
#include <cassert>

using namespace std;

vm_logger::vm_logger(string file_path, vm* v, int time_window_in_sec):file_path(file_path), owner_vm(v),
                                time_window_in_sec(time_window_in_sec), last_xs_ux_ts_ms(0){
	xs_path = owner_vm->xs_path+"/numa/log";
	vm_start_time_ms = ((long long)(owner_vm->start_time_sec_unix))*1000;
	// need to check initialization order
}

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


int vm_logger::ceiling_idx_of_in_mem_entries(long long unix_ts_ms){
	int lo = 0, hi = recent_entries.size()-1;
	if(recent_entries.empty() || unix_ts_ms < recent_entries[lo].first)
		return 0;
	if(unix_ts_ms > recent_entries[hi].first)
		return hi+1;
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
	return lo;
}

void vm_logger::insert_log_entry(long long unix_ts_ms, string log_entry){
	long long valid_unix_ts = time(0)*1000 - time_window_in_sec*1000;
	if(unix_ts_ms < valid_unix_ts)
		return;
	if(recent_entries.empty()){
		recent_entries.push_back({unix_ts_ms, log_entry});
	}
	else{
		int idx = ceiling_idx_of_in_mem_entries(unix_ts_ms);
		if(idx >= recent_entries.size())
			recent_entries.push_back({unix_ts_ms, log_entry});
		else
			recent_entries.insert(recent_entries.begin()+idx, {unix_ts_ms, log_entry});
	}
}

void vm_logger::flush_log_to_disk(bool flush_all){
	long long valid_unix_ts;
	if(flush_all)
       		valid_unix_ts = time(0)*1000;
	else
       		valid_unix_ts = time(0)*1000 - time_window_in_sec*1000;
	int idx = ceiling_idx_of_in_mem_entries(valid_unix_ts);
	for(int i=0; i< idx; i++){
		log_file << recent_entries[0].first << ", " << recent_entries[0].second << endl;
		recent_entries.erase(recent_entries.begin());
	}
}

void vm_logger::crawl_log_entries_from_xs(){
	//cout << "last_xs_ux_ts_ms: " << last_xs_ux_ts_ms << endl;
	//cout << "vm_start_time_ms: " << vm_start_time_ms << endl;
	auto new_records = get_new_log_entries(owner_vm->topod->xs, xs_path, 
			last_xs_ux_ts_ms, vm_start_time_ms);
	//cout << "last_xs_ux_ts_ms updated: " << last_xs_ux_ts_ms << endl;
	
	for(auto& x: new_records){
		//cout << "new_record len:" << x.second.size() << " content: " << x.second << " ts: " << x.first << endl;
		insert_log_entry(x.first, x.second);
	}
}
