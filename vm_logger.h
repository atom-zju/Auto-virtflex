#ifndef LOGGER_H
#define LOGGER_H

#include "vm.h"
#include <fstream>
/*
	vm log format:
		Header: <vm id>, <num_nodes>, <sample_interval>\n
		Content: ts, vCPU usage_node_0, bw_usage_node_0, vCPU usage_node_1, bw_usage_node_1 .... \n
		
*/

class vm_logger{
public:
	string file_path;
	ofstream log_file;
	//int num_nodes;
	vm *owner_vm;
	int time_window_in_sec;
	vector<pair<long long ,string>> recent_entries;
	
	vm_logger(string file_path, vm* v, int time_window_in_sec=10): file_path(file_path), owner_vm(v),
				time_window_in_sec(time_window_in_sec){}
	void init();
	int floor_idx_of_in_mem_entries(long long ts_unix_ms);
	void insert_log_entry(long long ts_unix_ms, string log_entry);
	void flush_log_to_disk(bool flush_all = false);
	~vm_logger();
};

#endif
