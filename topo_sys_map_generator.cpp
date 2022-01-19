#include "topo_sys_map_generator.h"
#include "runtime_estimator.h"
#include "performance_estimator.h"
#include "migration_cost_estimator.h"
#include "sys_map.h"
#include <cassert>

using namespace std;


topo_sys_map_generator::topo_sys_map_generator(topo_change_engine* topo_ce){
	assert(topo_ce);
	this->topo_ce = topo_ce;
}

int topo_sys_map_generator::generate_topo_sys_map(sys_map<int>& new_sys, topo_change_policy topo_cp){
	if(!topo_ce || policy_func_map.find(topo_cp) == policy_func_map.end())
		return -1;
	return policy_func_map[topo_cp](topo_ce, new_sys, topo_cp);
}

int max_topo_change_net_gain_func(topo_change_engine* topo_ce, sys_map<int>& new_sys, topo_change_policy topo_cp){
	auto topo_sys = (sys_map<int>*)topo_ce->get_sys_map(TOPO_SYS_MAP);
	auto topo_changeness_sys = (sys_map<int>*)topo_ce->get_sys_map(TOPO_CHANGENESS_SYS_MAP);
	auto shrink_node_rank_sys = (sys_map<int>*)topo_ce->get_sys_map(SHRINK_NODE_RANK_SYS_MAP);
	new_sys = *topo_sys;

	auto home_node_sys = (sys_map<int>*)topo_ce->get_sys_map(HOME_NODE_SYS_MAP);
	//home_node_assignment(topo_ce->topod, *topo_sys, *home_node_sys);
	//home_node_sys->update(topo_ce->topod);

	vector<int> vm_list;
	vector<int> topo_changeness_list;
	queue<int> free_pnode_q;
	int hi;
	topo_changeness_sys->sort_vm_by_sum_ascend(vm_list);
	for(auto& pnode_id: new_sys.pnode_list()){
		if(new_sys.pnode_sum(pnode_id) == 0)
			free_pnode_q.push(pnode_id);
	}
	for(auto& vm_id: vm_list){
		topo_changeness_list.push_back(topo_changeness_sys->vm_sum(vm_id));
	}
	// phase 1: natural trade, plus vm take away resources from minus vm
	// processing shrinking
	for(auto& vm_id: vm_list){
		if(topo_changeness_sys->vm_sum(vm_id) >= 0){
			break;
		}
		int vnode_id = shrink_node_rank_sys->max_vnode_in_vm(vm_id);
		shrink_node_rank_sys->vm_view(vm_id, vnode_id).data = -200;
		new_sys.vm_view(vm_id, vnode_id).data = 0;
		free_pnode_q.push(new_sys.vm_view(vm_id, vnode_id).pnode_id);
		cout << "shrinking vm " << vm_id << " vnode:" << vnode_id << endl;
	}
	// processing expansion
	for(hi=vm_list.size()-1; hi >= 0; hi--){
		cout << "considering vm_id :" << vm_list[hi] << endl;
		if(free_pnode_q.empty())
			break;
		if(topo_changeness_sys->vm_sum(vm_list[hi]) <= 0 )
			return 0;
		auto pnode_id = free_pnode_q.front();
		free_pnode_q.pop();
		int vnode_id = topo_ce->topod->pnode_to_vnode(vm_list[hi], pnode_id);
		new_sys.vm_view(vm_list[hi], vnode_id).data = 1;
		cout << "expanding vm " << vm_list[hi] << " vnode:" << vnode_id << endl;
	}
	// phase 2: extra trade (include preempt)
	int lo=0;
	while(lo < hi){
		if(topo_changeness_list[hi] <= 0)
			return 0;
		if(new_sys.vm_sum(vm_list[lo]) > 1){
			if(topo_changeness_list[lo] >= 0 && !topo_ce->can_preempt(vm_list[hi], vm_list[lo]))
				return 0;
			int lo_vnode_id = shrink_node_rank_sys->max_vnode_in_vm(vm_list[lo]);
                	new_sys.vm_view(vm_list[lo], lo_vnode_id).data = 0;
                	shrink_node_rank_sys->vm_view(vm_list[hi], lo_vnode_id).data = -200;
			cout << "shrinking vm " << vm_list[lo] << " vnode:" << lo_vnode_id << endl;
			int hi_vnode_id = topo_ce->topod->pnode_to_vnode(vm_list[hi],
					topo_ce->topod->vnode_to_pnode(vm_list[lo], lo_vnode_id));
                	new_sys.vm_view(vm_list[hi], hi_vnode_id).data = 1;
			cout << "expanding vm " << vm_list[hi] << " vnode:" << hi_vnode_id << endl;
			hi--;
		}
		else{
			lo++;
		}
	}
	// before going to phase 3, check is there any changes between old sys and new sys, 
	// if no, no need for phase 3
	bool topo_sys_map_diff = false;
	for(auto& vm_id: topo_sys->vm_list()){
		for(auto& vnode_id: topo_sys->vnode_list(vm_id)){
			if(new_sys.record_exist_vm_view(vm_id, vnode_id)
                        && new_sys.vm_view(vm_id, vnode_id).data != topo_sys->vm_view(vm_id, vnode_id).data)
                                topo_sys_map_diff = true;
		}
	}
	
	if(!topo_sys_map_diff)
		return 0;
	//return 0;
	// phase 3: considering the benefit and cost for topology change, approve or decline topo change.
	double benefit=0;
	double cost=0;
	for(auto& vm_id: topo_sys->vm_list()){
		int curr_num_node = topo_sys->vm_sum(vm_id);
		int new_num_node = new_sys.vm_sum(vm_id);
		float speedup=topo_ce->perf_esti->performance_gain_after_change(vm_id,curr_num_node,new_num_node);
		benefit += (speedup-1)*topo_ce->runtime_esti->remaining_runtime_in_sec(vm_id, curr_num_node);
	}	
	
	int migrate_cost_per_node_in_sec = 4;
	cost = migrate_cost_per_node_in_sec * topo_ce->migration_cost_esti->
		num_pages_to_be_migrated(*topo_sys, new_sys);

	// if the benefit outweight the cost, output 0 to approve the topo change, output -1 to decline.
	cout << "topo change benefit: " << benefit << ", cost: " << cost << endl; 
	//if(benefit > cost)
		return 0;
	//else{
	//	cout << "topo change declined" << endl;
	//	return -1;
	//}
}

//int topo_change_engine::generate_new_topo_map(sys_map<int>& new_sys){
//}


unordered_map<topo_change_policy, int(*)(topo_change_engine*, sys_map<int>&, topo_change_policy)> policy_func_map ={
	{max_topo_change_net_gain, max_topo_change_net_gain_func}
};
