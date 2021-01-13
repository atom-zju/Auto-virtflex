#ifndef VM_H
#define VM_H

#include <unordered_map>
#include "vnode.h"
//#include "topo_change_d.h"
using namespace std;

class topo_change_d;

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

	vnode* get_vnode_by_id(int id);
	int add_vnode(vnode*);
	int remove_vnode(int id);
	
	int shrink_vnode(int id);
	int expand_vnode(int id);
	
	void update_vnode_map(unsigned int ts);
	long average_bw_usage();
	
	vm(int id, topo_change_d* d,string s): xs_path(s), topod(d), vm_id(id), vcpu_path(s+"/cpu"){}
	~vm(); // free vnode_map pointers

};


#endif
