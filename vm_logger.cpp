#include "vm_logger.h"
#include "topo_change_d.h"
#include <cassert>

using namespace std;

vm_logger::~vm_logger(){
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

void vm_logger::log(){
	assert(owner_vm);
	assert(owner_vm->total_node > 0 );
	num_nodes =  owner_vm->total_node;
	log_file << owner_vm->ts;
	for(int i=0; i < num_nodes; i++){
		auto node = owner_vm->get_vnode_by_id(i);
		if(!node)
			log_file << ", -1, -1";
		else{
			log_file << ", " << node->get_recent_vcpu_usage() << ", " << node->average_bw_usage();
		}
	}
	log_file << endl;
}
