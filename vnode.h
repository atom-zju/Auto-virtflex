#ifndef VNODE_H
#define VNODE_H

#include "node.h"
#include "vm.h"

class vm;

class vnode: public node{
	vm* owner_vm;
	int vnode_id;

	void init_node();
	void updat_node();
	
	int shrink();
	int expand();

};

#endif 
