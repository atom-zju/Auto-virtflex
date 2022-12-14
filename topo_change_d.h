#ifndef TOPO_CHANGE_D_H
#define TOPO_CHANGE_D_H

#include<unordered_map>
#include<deque>
#include <vector>
#include "vm.h"
#include "pnode.h"
#include "util.h"

class topo_change_engine;
class sys_map_base;

class topo_change_event{
public:
	int vm_id;
	vector<int> vnode_list;
	int action; // -1: shrink; 1: expand
	topo_change_event(int id, int a, vector<int> n):vm_id(id), action(a), vnode_list(n){}
};

class topo_change_d {
	friend class vm;
	friend class vnode;
	friend class pnode;
	friend class cpu;
	friend class topo_change_engine;
	friend class vm_logger;
	friend class runtime_estimator;
	friend int topology_sys_map_update(topo_change_d*, sys_map_base*, long long);
	friend int home_node_sys_map_update(topo_change_d*, sys_map_base*, long long);
	friend int num_vcpu_sys_map_update(topo_change_d*, sys_map_base*, long long);
	friend int node_size_sys_map_update(topo_change_d*, sys_map_base*, long long);
	friend int undergo_topo_change_sys_map_update(topo_change_d*, sys_map_base*, long long);
private:
	unordered_map<int, vm*> vm_map;
	vector<pnode*> pnode_list;
	deque<topo_change_event> event_list;
	struct xs_handle *xs;
	xc_interface* xc_handle;
	libxl_ctx *xl_handle;
	xentoollog_logger_stdiostream *xl_logger;
	unsigned int interval_us;
	unsigned int ts; // timestamp
	topo_change_engine* engine;
	string xs_path;
       	bool interrupted;	
	
	vm* get_vm_by_id(int id);
	int add_vm(vm*);
	int remove_vm(int id);
	void process_event(topo_change_event& e);
	void register_pvnode(int vm_id, vnode* n, int pnode_id);
	void unregister_vnode(int vm_id, vnode* n, int pnode_id);
	long pnode_average_bw_usage(int pnode_id);
	int pnode_num_active_vnode(int pnode_id);
	long long pnode_cpu_usage(int pnode_id);
public:
	int shrink_vm(int id, int vnode_id);
	int expand_vm(int id, int vnode_id);

	vector<int> vm_list();
	string get_xs_path();
	int vnode_to_pnode(int vm_id, int vnode_id);
	int pnode_to_vnode(int vm_id, int pnode_id);
	int reserved_vnode_id(int vm_id);
	int set_reserved_vnode_id(int vm_id, int vnode_id);

	void init_pnode_list();

	void update_vm_map();
	void update_pnode_list();

	void set_interval_ms(unsigned int ms);
	void start();
	void interrupt();
	void generate_events();
	void process_events();
	
	topo_change_d();	
	~topo_change_d(); // free vm_map and pnode_list

};

#endif
