#include <iostream>
#include "sys_map.h"
#include "sample_queue.h"
#include "vm.h"
#include "topo_change_d.h"
#include "topo_change_engine.h"

int topology_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){
	assert(map->get_base_type() == "int");
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

int home_node_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){
	assert(map->get_base_type() == "int");
	auto smap = (sys_map<int>*)map;
	for(auto& vm: topod->vm_map){
                if(vm.first == 0)
                        continue;
		int home_node = vm.second->reserved_vnode_id;
                for(auto& vnode: vm.second->vnode_map){
                        int data = (vnode.second->vnode_id == home_node)? 1: 0;
                        int vm_id = vm.first;
                        int vnode_id = vnode.first;
                        int pnode_id = vnode.second->pnode_id;
                        map_record<int> m = map_record<int>(vm_id, vnode_id, pnode_id, data);
                        smap->push_back(m);
                }
        }
        return 0;
}

int node_size_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){
	assert(map->get_base_type() == "int");
	auto smap = (sys_map<int>*) map;
	for(auto& vm: topod->vm_map){
		if(vm.first == 0)
			continue;
		for(auto& vnode: vm.second->vnode_map){
			int data = vnode.second->node_size_in_mb();
			smap->push_back(map_record<int>(vm.first, vnode.first, vnode.second->pnode_id, data));
		}
	}
	return 0;	
}
int num_vcpu_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){
	assert(map->get_base_type() == "int");
	auto smap = (sys_map<int>*) map;	
	for(auto& vm: topod->vm_map){
		if(vm.first == 0)
			continue;
		for(auto& vnode: vm.second->vnode_map){
			int data = vnode.second->num_active_vcpu();
                        smap->push_back(map_record<int>(vm.first, vnode.first, vnode.second->pnode_id, data));
		}
	}
	return 0;
}

int vcpu_usage_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){

	//cout << "update vcpu usage sys map" << endl;
	assert(map->get_base_type() == "float");
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
			float data;
			if(since_ux_ts_ms < 0)
				data = sample_queue<float>::last_record_avg(
						data_map[vm_id][vnode_id][CPU_USAGE_SQ]);
			else
				data = sample_queue<float>::calculate_avg(
					data_map[vm_id][vnode_id][CPU_USAGE_SQ], since_ux_ts_ms);
			//cout << "data: " << data << endl;
			smap->push_back(map_record<float>(vm_id, vnode_id, pnode_id, data));
			//cout << "dataaaaa: " << smap->records[smap->records.size()-1].data << endl;
		}
	return 0;
}


int bw_usage_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){

	cout << "update bw usage sys map" << endl;
	assert(map->get_base_type() == "int");
	auto smap = (sys_map<int>*)map;
	auto& data_map = sample_queue<int>::data_map;
	for(auto& vm_id: sample_queue<int>::vm_list())
		for(auto& vnode_id: sample_queue<int>::vnode_list(vm_id)){
			//cout << "vm: " << vm_id << ", vnode: " << vnode_id<<endl;
			if(data_map[vm_id][vnode_id].find(BW_USAGE_SQ) == data_map[vm_id][vnode_id].end())
				continue;
			//cout << "valid bw queue found" << endl;
			int pnode_id = topod->vnode_to_pnode(vm_id, vnode_id);
			assert(pnode_id >= 0);
			//cout << "sq vector.size: " << data_map[vm_id][vnode_id][BW_USAGE_SQ].size() << endl;
			//data_map[vm_id][vnode_id][BW_USAGE_SQ][0]->print();
			int data;
			if(since_ux_ts_ms < 0)
				data = sample_queue<int>::last_record_avg(
                                        data_map[vm_id][vnode_id][BW_USAGE_SQ]);
			else
				data = sample_queue<int>::calculate_avg(
					data_map[vm_id][vnode_id][BW_USAGE_SQ], since_ux_ts_ms);
			//cout << "data: " << data << endl;
			smap->push_back(map_record<int>(vm_id, vnode_id, pnode_id, data));
			//cout << "dataaaaa: " << smap->records[smap->records.size()-1].data << endl;
		}
	return 0;
}

int num_thread_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){
	assert(map->get_base_type() == "int");
	auto smap = (sys_map<int>*)map;
        auto& data_map = sample_queue<int>::data_map;
	for(auto& vm_id: sample_queue<int>::vm_list()){
		if(!sample_queue<int>::has_sys_node(vm_id) ||
			data_map[vm_id][SYS_NODE_ID].find(NUM_OF_THREAD_SQ) == data_map[vm_id][SYS_NODE_ID].end())
			continue;
		int data;
                if(since_ux_ts_ms < 0)
                        data = sample_queue<int>::last_record_avg(
                               data_map[vm_id][SYS_NODE_ID][NUM_OF_THREAD_SQ]);
                else
                        data = sample_queue<int>::calculate_avg(
                               data_map[vm_id][SYS_NODE_ID][NUM_OF_THREAD_SQ], since_ux_ts_ms);
		smap->push_back(map_record<int>(vm_id, SYS_NODE_ID, SYS_NODE_ID, data));
	}
	return 0;
}

int idle_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){
	assert(map->get_base_type() == "int");
        auto smap = (sys_map<int>*)map;
	auto vcpu_usage_sys = (sys_map<float>*) smap->topoe->get_sys_map(VCPU_USAGE_SYS_MAP);
	float idle_thres = 0.05;
	for(auto& vm_id: vcpu_usage_sys->vm_list()){
		int data = vcpu_usage_sys->vm_sum(vm_id) > idle_thres? 1:0;
		smap->push_back(map_record<int>(vm_id, SYS_NODE_ID, SYS_NODE_ID, data));
	}
	return 0;
}

int topo_changeness_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){
	assert(map->get_base_type() == "int");
	auto smap = (sys_map<int>*)map;
	auto topoe = smap->topoe;
	unordered_map<int, int> scores;
	auto idle_sys = (sys_map<int>*) smap->topoe->get_sys_map(IDLE_SYS_MAP);
	auto bw_sys = (sys_map<int>*) smap->topoe->get_sys_map(BW_USAGE_SYS_MAP);
	auto topo_sys = (sys_map<int>*) smap->topoe->get_sys_map(TOPO_SYS_MAP);
	auto num_thread_sys = (sys_map<int>*) smap->topoe->get_sys_map(NUM_THREAD_SYS_MAP);
	auto num_vcpu_sys = (sys_map<int>*) smap->topoe->get_sys_map(NUM_VCPU_SYS_MAP);
	
	int intact_range = 10;
	
	// idle scores
	int idle_score = -200;
	for(auto& vm_id: idle_sys->vm_list()){
		if(idle_sys->vm_view(vm_id, SYS_NODE_ID).data == 0){
			scores[vm_id]+= idle_score;
		}
	}
	for(auto& vm_id: topo_sys->vm_list()){
		scores[vm_id] += 2-topo_sys->vm_sum(vm_id);
	}
	
	// bw scores
	int bw_high_thres = 700;
	int bw_low_thres = 200;
	int bw_high_score = 100;
	int bw_low_score = -100;
	for(auto& vm_id: bw_sys->vm_list()){
                int bw = bw_sys->vm_avg(vm_id, topo_sys);
		if( bw >= bw_high_thres){
                        scores[vm_id]+= bw_high_score;
                }
		else if(bw <= bw_low_thres){
			scores[vm_id]+= bw_low_score;
		}
        }

	// num of thread scores
	int num_of_thread_score = 50;
	for(auto& vm_id: num_vcpu_sys->vm_list()){
		if(num_vcpu_sys->vm_sum(vm_id) < num_thread_sys->vm_sum(vm_id)){
			scores[vm_id] += num_of_thread_score;
		}
	}

	// guest specified workload attr: threads
	int workload_attr_thread_score = 50;
	for(auto& vm_id: num_vcpu_sys->vm_list()){
		if(!topoe->wlattr->attr_exist(vm_id, "thread"))
			continue;
		int wl_nth = topoe->wlattr->query_attr(vm_id, "thread");
		if(num_vcpu_sys->vm_sum(vm_id) < wl_nth)
			scores[vm_id] += workload_attr_thread_score;
	}

	// post-processing scores	
	for(auto&& x: scores){
		// nuturalize small numbers
		if(abs(x.second) <= intact_range){
			x.second = 0;
		}
		// zero for vm that already reduced to 1 node
		if(x.second < 0 && topo_sys->vm_sum(x.first) <= 1)
			x.second = 0;
	}

	// print 
	for(auto& vm_id: topo_sys->vm_list()){
		smap->push_back(map_record<int>(vm_id, SYS_NODE_ID, SYS_NODE_ID, scores[vm_id]));
	}
	return 0;
	
}

int shrink_node_rank_sys_map_update(topo_change_d* topod, sys_map_base* map, long long since_ux_ts_ms){
	assert(map->get_base_type() == "int");
        auto smap = (sys_map<int>*)map;
	auto topo_sys_ptr = (sys_map<int>*) smap->topoe->get_sys_map(TOPO_SYS_MAP);
	auto topo_sys = *(topo_sys_ptr); // use a copy of topo_sys because the content might be modified
	auto changeness_sys = (sys_map<int>*) smap->topoe->get_sys_map(TOPO_CHANGENESS_SYS_MAP);
	auto home_node_sys = (sys_map<int>*) smap->topoe->get_sys_map(HOME_NODE_SYS_MAP);

	unordered_map<int, int> scores;
	for(auto& vm_id: changeness_sys->vm_list()){
		scores.clear();
		for(auto& vnode_id: topo_sys.vnode_list(vm_id)){
			int pnode_id = topod->vnode_to_pnode(vm_id, vnode_id);
			scores[vnode_id] = 0;
			// if home node, lower the shrink priority
			if(home_node_sys->vm_view(vm_id, vnode_id).data == 1 ||
					topo_sys.vm_view(vm_id, vnode_id).data == 0){
				scores[vnode_id] += -200;
			}
			else{
				// if shared with other vms, higher the shrink priority
				int share_degree = topo_sys.pnode_sum(pnode_id);
				if(share_degree > 1 ){
		       			scores[vnode_id] += share_degree*50;
					topo_sys.vm_view(vm_id, vnode_id).data = 0;
				}
			}
			smap->push_back(map_record<int>(vm_id, vnode_id, pnode_id, scores[vnode_id]));
		}
	}	

	return 0;
}

unordered_map<string, pair<int (*)(topo_change_d*, sys_map_base*, long long), string>> sys_map_info_map{
	{TOPO_SYS_MAP, {topology_sys_map_update, "int"}},
	{VCPU_USAGE_SYS_MAP, {vcpu_usage_sys_map_update, "float"}},
	{BW_USAGE_SYS_MAP, {bw_usage_sys_map_update, "int"}}, 
	{HOME_NODE_SYS_MAP, {home_node_sys_map_update, "int"}},
	{NUM_THREAD_SYS_MAP, {num_thread_sys_map_update, "int"}},
	{NODE_SIZE_SYS_MAP, {node_size_sys_map_update, "int"}},
	{NUM_VCPU_SYS_MAP, {num_vcpu_sys_map_update, "int"}},
	///////////////////////////////////////////
	{IDLE_SYS_MAP, {idle_sys_map_update, "int"}},
	{TOPO_CHANGENESS_SYS_MAP, {topo_changeness_sys_map_update, "int"}},
	{SHRINK_NODE_RANK_SYS_MAP, {shrink_node_rank_sys_map_update, "int"}}
};
