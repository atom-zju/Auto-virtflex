#ifndef PNODE_H
#define PNODE_H
#include <vector>
#include "node.h"
#include "vnode.h"

using namespace std;

class pnode: public node {
	vector<vnode*> vnode_list;
	pnode(int id):node(id) {}
		
};

#endif
