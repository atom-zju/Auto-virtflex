#ifndef VM_H
#define VM_H

#include <unordered_map>
#include "vnode.h"
//#include "vm_logger.h"
//#include "topo_change_d.h"
using namespace std;

class topo_change_d;
class vm_logger;

class vm{

public:
	int vm_id;
	unordered_map<int, vnode*> vnode_map;
	string xs_path;
	string vcpu_path;
	int total_node;
	int active_node;
	topo_change_d* topod;
	unsigned int ts;
	uint32_t start_time_sec_unix;
	vm_logger* logger;
	sample_queue* num_thread_sampleq;

        vector<deque<pair<long long, int>>> num_thread_sample;
        int max_sample_size;

	vnode* get_vnode_by_id(int id);
	int add_vnode(vnode*);
	int remove_vnode(int id);
	
	int shrink_vnode(int id);
	int expand_vnode(int id);
	
	void update_vnode_map(unsigned int ts);
	long average_bw_usage();
	float get_average_vcpu_load();
	
	void active_node_list(vector<int>& v);
	void inactive_node_list(vector<int>& v);
	
	vm(int id, topo_change_d* d,string s);
	~vm(); // free vnode_map pointers

};


#endif
