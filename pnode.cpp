#include "pnode.h"
#include <iostream>
#include <cassert>

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
void pnode::unregister_vnode(int vm_id, vnode* n){
	if(vnode_map.find(vm_id) != vnode_map.end()){
		assert(vnode_map[vm_id] == n);
		vnode_map.erase(vm_id);
	}
}

void pnode::update_vnode_map(int ts){
	cout << "updating pnode, id: " << pnode_id << endl;
	active_vnodes = 0;
	total_vnodes = 0;
	for(auto& x: vnode_map){
		if(x.second->ts < ts){
			if(x.second == owner_vnode)
				owner_vnode = NULL;
			vnode_map.erase(x.first);
		}
		else{
			total_vnodes++;
			if(x.second->enabled){
				active_vnodes++;
			}
			else{
				if(x.second == owner_vnode)
					owner_vnode = NULL;
			}
		}
	}
	cout << "total_vnodes: " << total_vnodes << endl;
	cout << "active_vnodes: " << active_vnodes << endl;
	
	if(owner_vnode == NULL){
		pick_owner_vnode();
	}
	if(owner_vnode)
		get_owner_vnode_stat();
		

}

void pnode::pick_owner_vnode(){
	// right now just randomly pick an enabled node
	assert(!owner_vnode);
	for(auto& x: vnode_map){
		if(x.first != 0 && x.second->enabled){
			owner_vnode = x.second;
			return;
		}
	}
}

void pnode::get_owner_vnode_stat(){
	// right now the only stat to get is bw usage
	assert(owner_vnode);
	owner_vnode->read_bw_usage_from_xs();
	copy_owner_vnode_bw_usage();
}

void pnode::copy_owner_vnode_bw_usage(){
	assert(owner_vnode);
	bw_rd = owner_vnode->bw_rd;
	bw_wr = owner_vnode->bw_wr;
}

long pnode::average_bw_usage(){
	if(!owner_vnode)
		return -1;
	assert(bw_rd.size() == bw_wr.size());
        if(bw_rd.empty())
                return -1;

        long bw_usage = 0;
        for(auto& x: bw_rd){
                bw_usage+=x;
        }
        for(auto& x: bw_wr){
                bw_usage+=x;
        }
        return bw_usage/(bw_rd.size()+ bw_wr.size());
}
