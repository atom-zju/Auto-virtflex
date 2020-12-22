#ifndef NODE_H
#define NODE_H

#include <set>
#include "cpu.h"

using namespace std; 

class node{
public:
	int pnode_id;
	int capacity;
	set<cpu> cpu_set;
	int bw_usage;
	node(int id): pnode_id(id) {}
	node(){}

};


#endif
