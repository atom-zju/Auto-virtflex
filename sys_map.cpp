#include <iostream>
#include "sys_map.h"
#include "sample_queue.h"
#include "vm.h"
#include "topo_change_d.h"

int topology_sys_map_update(topo_change_d* topod, void* map, long long since_ux_ts_ms){
	auto smap = (sys_map<int>*)map;
	for(auto& vm: topod->vm_map){
		if(vm.first == 0)
			continue;
		for(auto& vnode: vm.second->vnode_map){
			int data = vnode.second->enabled? 1: 0;
			int vm_id = vm.first;
			int vnode_id = vnode.first;
			int pnode_id = vnode.second->pnode_id;
			map_record<int> m = map_record<int>(vm_id, vnode_id, pnode_id, data);
			smap->push_back(m);
		}
	}
	return 0;	
}

int vcpu_usage_sys_map_update(topo_change_d* topod, void* map, long long since_ux_ts_ms){

	//cout << "update vcpu usage sys map" << endl;
	auto smap = (sys_map<float>*)map;
	auto& data_map = sample_queue<float>::data_map;
	for(auto& vm_id: sample_queue<float>::vm_list())
		for(auto& vnode_id: sample_queue<float>::vnode_list(vm_id)){
			//cout << "vm: " << vm_id << ", vnode: " << vnode_id<<endl;
			if(data_map[vm_id][vnode_id].find(CPU_USAGE_SQ) == data_map[vm_id][vnode_id].end())
				continue;
			//cout << "valid vcpu queue found" << endl;
			int pnode_id = topod->vnode_to_pnode(vm_id, vnode_id);
			assert(pnode_id >= 0);
			float data = sample_queue<float>::calculate_avg(
					data_map[vm_id][vnode_id][CPU_USAGE_SQ], since_ux_ts_ms);
			//cout << "data: " << data << endl;
			smap->push_back(map_record<float>(vm_id, vnode_id, pnode_id, data));
			//cout << "dataaaaa: " << smap->records[smap->records.size()-1].data << endl;
		}
	return 0;
}


int bw_usage_sys_map_update(topo_change_d* topod, void* map, long long since_ux_ts_ms){

	cout << "update bw usage sys map" << endl;
	auto smap = (sys_map<int>*)map;
	auto& data_map = sample_queue<int>::data_map;
	for(auto& vm_id: sample_queue<int>::vm_list())
		for(auto& vnode_id: sample_queue<int>::vnode_list(vm_id)){
			cout << "vm: " << vm_id << ", vnode: " << vnode_id<<endl;
			if(data_map[vm_id][vnode_id].find(BW_USAGE_SQ) == data_map[vm_id][vnode_id].end())
				continue;
			//cout << "valid bw queue found" << endl;
			int pnode_id = topod->vnode_to_pnode(vm_id, vnode_id);
			assert(pnode_id >= 0);
			//cout << "sq vector.size: " << data_map[vm_id][vnode_id][BW_USAGE_SQ].size() << endl;
			//data_map[vm_id][vnode_id][BW_USAGE_SQ][0]->print();
			int data = sample_queue<int>::calculate_avg(
					data_map[vm_id][vnode_id][BW_USAGE_SQ], since_ux_ts_ms);
			cout << "data: " << data << endl;
			smap->push_back(map_record<int>(vm_id, vnode_id, pnode_id, data));
			//cout << "dataaaaa: " << smap->records[smap->records.size()-1].data << endl;
		}
	return 0;
}

unordered_map<string, int (*)(topo_change_d*, void*, long long)> sys_map_func_map{
	{TOPO_SYS_MAP, topology_sys_map_update},
	{VCPU_USAGE_SYS_MAP, vcpu_usage_sys_map_update},
	{BW_USAGE_SYS_MAP, bw_usage_sys_map_update}
};