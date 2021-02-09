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
	recent_cpu_usage = 0;
	for(auto& x: vnode_map){
		if(x.second->ts < ts){
			if(x.second == owner_vnode)
				owner_vnode = NULL;
			vnode_map.erase(x.first);
		}
		else{
			total_vnodes++;
			recent_cpu_usage += x.second->get_recent_vcpu_usage();
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
	bw_rd_channel_sample = owner_vnode->bw_rd_channel_sample;
	bw_wr_channel_sample = owner_vnode->bw_wr_channel_sample;
}

long pnode::average_bw_usage(){
	if(!owner_vnode)
		return -1;
	if(bw_rd_channel_sample.empty() || bw_wr_channel_sample.empty()
		|| bw_rd_channel_sample[0].empty() || bw_wr_channel_sample[0].empty()){
		return -1;
	}
	long long valid_ts_ms = bw_rd_channel_sample[0].back().first - VALID_SAMPLE_INTERVAL_MS;
	int valid_count = 0;
	long sum = 0;
	for(auto& y: bw_rd_channel_sample){
		for(auto& x: y){
			if (x.first >= valid_ts_ms){
				valid_count++;
				sum+=x.second;
			}
		}
	}

        for(auto& y: bw_wr_channel_sample){
		for(auto& x: y){
                	if (x.first >= valid_ts_ms){
                        	valid_count++;
                        	sum+=x.second;
                	}
		}
        }

	if(valid_count == 0)
		return -1;
	return sum/valid_count;
}
