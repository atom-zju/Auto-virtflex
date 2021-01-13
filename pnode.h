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
	unordered_map<int, vnode*> vnode_map;
	pnode(int id):node(id) {}
	~pnode();		
	void register_vnode(int vm_id, vnode* n);
	void update_vnode_map(int ts);
};

#endif
