#ifndef VM_H
#define VM_H

#include <unordered_map>
#include "vnode.h"
//#include "vm_logger.h"
//#include "topo_change_d.h"
using namespace std;

class topo_change_d;
class vm_logger;
class topo_change_status;

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
	topo_change_status* tc_status;
	sample_queue<int>* num_thread_sampleq;
	sample_queue<int>* idleness_sampleq;
	sample_queue<int>* num_active_node_sampleq;

	int topo_changeness;
	int reserved_vnode_id;

        vector<deque<pair<long long, int>>> num_thread_sample;
        int max_sample_size;

	bool is_running_workload();

	vnode* get_vnode_by_id(int id);
	int add_vnode(vnode*);
	int remove_vnode(int id);
	
	int shrink_vnode(int id);
	int expand_vnode(int id);
	
	void update_vnode_map(unsigned int ts);
	
	long average_bw_usage();
	float get_average_vcpu_load();
	
	//topology changeness
	void calculate_topo_changeness();
	
	void active_node_list(vector<int>& v);
	void inactive_node_list(vector<int>& v);
	
	vm(int id, topo_change_d* d,string s);
	~vm(); // free vnode_map pointers

};


#endif
