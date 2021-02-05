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
	int num_nodes;
	vm *owner_vm;
	
	vm_logger(string file_path, vm* v): file_path(file_path), owner_vm(v){}
	void init();
	void log();
	~vm_logger();
};

#endif
