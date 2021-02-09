#include<iostream>
#include<vector>
#include<cassert>
#include "vm.h"
#include "vm_logger.h"
#include "topo_change_d.h"

using namespace std;

vm::vm(int id, topo_change_d* d,string s): xs_path(s), topod(d), vm_id(id), vcpu_path(s+"/cpu"){
	logger = new vm_logger("log/vm_"+to_string(vm_id)+"_log.txt", this);
	assert(logger);
	logger->init();
}

vm::~vm(){
	assert(topod);
	for(auto& x: vnode_map){
		topod->unregister_vnode(vm_id, x.second, x.second->pnode_id);
		delete x.second;
	}
	if(logger)
		delete logger;
	cout << "vm: " << vm_id << " terminated." << endl;
}

vnode* vm::get_vnode_by_id(int id){
	if(vnode_map.find(id) != vnode_map.end()){
		return vnode_map[id];
	}
	return NULL;
}

void vm::update_vnode_map(unsigned int ts){
	this->ts = ts;
	vector<string> dir;
	string node_path = string(xs_path).append("/numa/node");
	assert(topod);
	list_xenstore_directory(topod->xs, node_path, dir);
	
	cout << node_path <<endl;
	for(auto& x: dir){
		//cout<< "\t"<< x << endl;
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
	total_node = 0;
	active_node = 0;
	for(auto& x: vnode_map){
		auto v = x.second;
		if(v->ts < ts){
			delete v;
			vnode_map.erase(x.first);
		}
		else{
			total_node++;
			if(v->enabled)
				active_node++;
		}
	}
}


int vm::shrink_vnode(int id){
	vnode* v = get_vnode_by_id(id);
	if(!v){
		cout<< "Didn't find vnode " << id << " in vm::shrink_vnode" <<endl;
		return -1;
	}
	return v->shrink();
}
int vm::expand_vnode(int id){
	vnode* v = get_vnode_by_id(id);
        if(!v){
                cout<< "Didn't find vnode " << id << " in vm::expand_vnode" <<endl;
		return -1;
	}
        return v->expand();
}

long vm::average_bw_usage(){
	long bw_usage = 0;
	long cnt = 0;

	for(auto& x: vnode_map){
		if(!(x.second->enabled))
			continue;
		long tmp = x.second->average_bw_usage();
		if(tmp >= 0 ){
			bw_usage += tmp;
			cnt++;
		}
	}
	return cnt>0? bw_usage/cnt: -1;
	
}

void vm::active_node_list(vector<int>& v){
	assert(v.empty());
	for(auto& x: vnode_map){
		if(x.second->enabled){
			v.push_back(x.first);
		}
	}
}

void vm::inactive_node_list(vector<int>& v){
	assert(v.empty());
	for(auto& x: vnode_map){
		if(!(x.second->enabled)){
			v.push_back(x.first);
		}
	}
}
