#ifndef NODE_H
#define NODE_H

#include <set>
#include "cpu.h"

using namespace std; 

class node{
	int pnode_id;
	int capacity;
	set<cpu> vcpu_set;
	int bw_usage;

};


#endif
