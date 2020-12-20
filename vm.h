#ifndef VM_H
#define VM_H

#include <unordered_map>
#include "vnode.h"

using namespace std;

class vm{

public:
	int vm_id;
	unordered_map<int, vnode*> vnode_map;
	
	vnode* get_vnode_by_id(int id);
	int add_vnode(vnode*);
	int remove_vnode(int id);
	
	int shrink_vnode(int id);
	int expand_vnode(int id);
	
	void init_vnode_map();
	void update_vnode_map();
	
	~vm(); // free vnode_map pointers

};


#endif
