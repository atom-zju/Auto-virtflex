#ifndef TOPO_CHANGE_D_H
#define TOPO_CHANGE_D_H

#include<unordered_map>
#include "vm.h"
#include "pnode.h"
#include "util.h"

class topo_change_event{
public:
	int vm_id;
	int vnode_id;
	int action; // -1: shrink; 1: expand
};

class topo_change_d {
	friend class vm;
	friend class vnode;
private:
	unordered_map<int, vm*> vm_map;
	vector<pnode*> pnode_list;
	vector<topo_change_event> event_list;
	struct xs_handle *xs;
	unsigned int ts; // timestamp
	
	vm* get_vm_by_id(int id);
	int add_vm(vm*);
	int remove_vm(int id);

	int shrink_vm(int id, int vnode_id);
	int expand_vm(int id, int vnode_id);
public:
	void init_pnode_list();

	void update_vm_map();
	void update_pnode_list();

	void generate_events();
	void process_events();
	
	topo_change_d();	
	~topo_change_d(); // free vm_map and pnode_list

};

#endif
