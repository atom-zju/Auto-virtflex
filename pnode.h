#ifndef PNODE_H
#define PNODE_H
#include <unordered_map>
#include "node.h"
#include "vnode.h"

using namespace std;

class pnode: public node {
public:
	int total_vnodes;
	int active_vnodes;
	long long recent_cpu_usage;
	vnode* owner_vnode;
	unordered_map<int, vnode*> vnode_map;
	pnode(int id):node(id), owner_vnode(NULL) {}
	~pnode();		
	void register_vnode(int vm_id, vnode* n);
	void unregister_vnode(int vm_id, vnode* n);
	void update_vnode_map(int ts);
	void pick_owner_vnode(void);
	void get_owner_vnode_stat(void);
	long average_bw_usage();
	void copy_owner_vnode_bw_usage();
};

#endif
