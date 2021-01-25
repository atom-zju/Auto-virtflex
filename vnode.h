#ifndef VNODE_H
#define VNODE_H

#include <string>
#include "node.h"

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
	
	int shrink();
	int expand();
	
	long average_bw_usage();
	int active_nodes_in_pnode();
	void read_bw_usage_from_xs();
	int change_pnode_owner_xs(bool own);

	vnode(int vnode_id, string path, vm* o, topo_change_d* t): 
		vnode_id(vnode_id), xs_path(path), topod(t), enabled(1), owner_vm(o){}

};

#endif 
