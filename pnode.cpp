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
	/* 1. create sample queue
	   2. merge samples from vnode sample queue */
	int last_channel = bw_rd_channel_sample.size()-1;
	while(bw_rd_channel_sample.size() < owner_vnode->bw_rd_channel_sample.size()){
		bw_rd_channel_sample.push_back(new sample_queue(
				owner_vnode->bw_rd_channel_sample[last_channel+1]->name,
				NULL,
				owner_vnode->bw_rd_channel_sample[last_channel+1]->name));	
		last_channel++;
	}
	for(int i=0; i < bw_rd_channel_sample.size() && i < owner_vnode->bw_rd_channel_sample.size(); i++){
		assert(bw_rd_channel_sample[i]);
		bw_rd_channel_sample[i]->merge_sample_queue(
				owner_vnode->bw_rd_channel_sample[i]);
	}
	last_channel = bw_wr_channel_sample.size()-1;
	while(bw_wr_channel_sample.size() < owner_vnode->bw_wr_channel_sample.size()){
                bw_wr_channel_sample.push_back(new sample_queue(
				owner_vnode->bw_wr_channel_sample[last_channel+1]->name,
				NULL,
				owner_vnode->bw_wr_channel_sample[last_channel+1]->name));
		last_channel++;
        }
        for(int i=0; i < bw_wr_channel_sample.size() && i < owner_vnode->bw_wr_channel_sample.size(); i++){
		assert(bw_wr_channel_sample[i]);
                bw_wr_channel_sample[i]->merge_sample_queue(
                                owner_vnode->bw_wr_channel_sample[i]);
        }

}

long pnode::average_bw_usage(){
	if(!owner_vnode)
		return -1;
	if(bw_rd_channel_sample.empty() || bw_wr_channel_sample.empty()){
		return -1;
	}
	int sample_count = 0;
	long sum = 0;
	long long valid_ts_ms = ((long long)time(0))*1000 - VALID_SAMPLE_INTERVAL_MS;
	for(auto& y: bw_rd_channel_sample){
		int avg = y->average_value_since_ts(valid_ts_ms);
		if(avg >= 0){
			sum+= avg;
			sample_count++;
		}
	}

        for(auto& y: bw_wr_channel_sample){
		int avg = y->average_value_since_ts(valid_ts_ms);
		if(avg >= 0){
			sum+= avg;
			sample_count++;
		}
        }

	if(sample_count == 0)
		return -1;
	return sum/sample_count;
}
