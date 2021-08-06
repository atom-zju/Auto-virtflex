#ifndef VNODE_H
#define VNODE_H

#include <string>
#include "node.h"
#include "topo_change.h"

class vm;
class topo_change_d;

class vnode: public node{
public:
	vm* owner_vm;
	int vnode_id;
	int target;
	int low_target;
	bool enabled;
	string xs_path;
	topo_change_d* topod;

	void update_node(unsigned int ts);
	
	int shrink(int reserved_vnode_id);
	int expand(int reserved_vnode_id);
	
	long average_bw_usage();
	long long get_recent_vcpu_usage();
	float get_average_vcpu_load();
	int active_nodes_in_pnode();
	//void read_bw_usage_from_xs();
	void clear_bw_sq();
	int change_pnode_owner_xs(bool own);
	
	// topology changeness
	int get_topo_changeness();
	long long bw_long_average();
	long long bw_short_average();
	void calculate_bw_sample_queue_averages();

	vnode(int vnode_id, string path, vm* o, topo_change_d* t): 
		vnode_id(vnode_id), xs_path(path), topod(t), enabled(1), owner_vm(o){}

};

#endif 
