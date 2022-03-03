#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include "vm.h"
/*
	vm log format:
		Header: <vm id>, <num_nodes>, <sample_interval>\n
		Content: ts, vCPU usage_node_0, bw_usage_node_0, vCPU usage_node_1, bw_usage_node_1 .... \n
		
*/

class vm_logger{
private:
	string file_path;
	ofstream log_file;
	vm *owner_vm;
	string xs_path;
	long long last_xs_ux_ts_ms;
	long long vm_start_time_ms;
	int time_window_in_sec;
	vector<pair<long long ,string>> recent_entries;
	
	int ceiling_idx_of_in_mem_entries(long long ts_unix_ms);
public:
	vm_logger(string file_path, vm* v, int time_window_in_sec=10);
	~vm_logger();
	void init();
	void insert_log_entry(long long ts_unix_ms, string log_entry);
	void flush_log_to_disk(bool flush_all = false);
	void crawl_log_entries_from_xs();
};

#endif
