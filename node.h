#ifndef NODE_H
#define NODE_H

#include <set>
#include <vector>
#include "cpu.h"

using namespace std; 

class node{
public:
	int pnode_id;
	int capacity;
	set<cpu> cpu_set;
	vector<int> bw_rd;
	vector<int> bw_wr;
	unsigned int ts;

	node(int id): pnode_id(id) {}
	node() {}

};


#endif
