#include<iostream>
#include<vector>
#include "vm.h"
#include "topo_change_d.h"

using namespace std;

vm::~vm(){
	for(auto& x: vnode_map){
		delete x.second;
	}
	cout << "vm: " << vm_id << " terminated." << endl;
}

void vm::update_vnode_map(unsigned int ts){
	this->ts = ts;
	vector<string> dir;
	string node_path = string(xs_path).append("/numa/node");
	list_xenstore_directory(topod->xs, node_path, dir);
	
	cout << node_path <<endl;
	for(auto& x: dir){
		cout<< "\t"<< x << endl;
		int node_id = stoi(x);
		if(vnode_map.find(node_id) == vnode_map.end()){
			vnode* v =  new vnode(node_id, string(node_path).append("/").append(x), this, topod);
			v->update_node(ts);
			vnode_map[node_id] = v;
		}
		else{
		// if node already in vnode_map
			vnode_map[node_id]->update_node(ts);
		}
		
	}

	// delete obsolete vnode
	for(auto& x: vnode_map){
		auto v = x.second;
		if(v->ts < ts){
			delete v;
			vnode_map.erase(x.first);
		}
	}
}
