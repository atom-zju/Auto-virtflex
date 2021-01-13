#include "pnode.h"
#include <iostream>

pnode::~pnode(){
	for(auto& x: vnode_map){
		delete x.second;
	}
}

void pnode::register_vnode(int vm_id, vnode* n){
	if(vnode_map.find(vm_id) == vnode_map.end()){
		vnode_map[vm_id] = n;
	}
}

void pnode::update_vnode_map(int ts){
	cout << "updating pnode, id: " << pnode_id << endl;
	active_vnodes = 0;
	total_vnodes = 0;
	for(auto& x: vnode_map){
		if(x.second->ts < ts){
			vnode_map.erase(x.first);
		}
		else{
			total_vnodes++;
			if(x.second->enabled){
				active_vnodes++;
			}
		}
	}
	cout << "total_vnodes: " << total_vnodes << endl;
	cout << "active_vnodes: " << active_vnodes << endl;
}
