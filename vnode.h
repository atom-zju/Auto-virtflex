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
	string xs_path;
	topo_change_d* topod;

	void update_node(unsigned int ts);
	
	int shrink();
	int expand();

	vnode(int vnode_id, string path, vm* o, topo_change_d* t): 
		vnode_id(vnode_id), xs_path(path), topod(t), owner_vm(o){}

};

#endif 
