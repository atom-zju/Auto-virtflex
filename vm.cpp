#include<iostream>
#include<vector>
#include "vm.h"
#include "topo_change_d.h"

using namespace std;

vm::~vm(){
	for(auto& x: vnode_map){
		delete x.second;
	}
}

void vm::init_vnode_map(){
	vector<string> dir;
	string node_path = string(xs_path).append("/numa/node");
	list_xenstore_directory(topod->xs, node_path, dir);
	
	cout << node_path <<endl;
	for(auto& x: dir){
		cout<< "\t"<< x << endl;
		int node_id = stoi(x);
		if(vnode_map.find(node_id) == vnode_map.end()){
			vnode* v =  new vnode(node_id, string(node_path).append("/").append(x), this, topod);
			v->init_node();
			vnode_map[node_id] = v;
		}
		
	}
}
