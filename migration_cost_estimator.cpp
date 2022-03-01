#include "migration_cost_estimator.h"
#include "sys_map.h"

int migration_cost_estimator::num_pages_to_be_migrated(sys_map<int>& old_sys, sys_map<int>& new_sys){
	// assuming no node sharing,
	// for current implementation, output is the number of nodes that has been changed instead of the
	// number of pages to be migrated
	int cnt = 0;
	for(auto& vm_id: old_sys.vm_list()){
		for(auto& vnode_id: old_sys.vnode_list(vm_id)){
			if(new_sys.record_exist_vm_view(vm_id, vnode_id) 
			&& new_sys.vm_view(vm_id, vnode_id).data != old_sys.vm_view(vm_id, vnode_id).data)
				cnt++;
		}
	}
	return cnt;
}

int migration_cost_estimator::default_topo_change_estimate_ms(int vm_id, int from_num_node, 
		int to_num_node){
	if(from_num_node == to_num_node)
		return 0;
	else if (from_num_node < to_num_node)
		return (to_num_node - from_num_node)*default_add_cost_per_node_ms;
	else
		return (from_num_node - to_num_node)*default_remove_cost_per_node_ms;
}

void migration_cost_estimator::insert_topo_change_cost_esti_entry(int vm_id, int from_num_node,
                                int to_num_node, workload_feature wl_feat, int cost_in_ms){
	est_map[vm_id][{from_num_node, to_num_node}].push_back({wl_feat, cost_in_ms});
}

int migration_cost_estimator::topo_change_cost_estimate_ms(int vm_id, int from_num_node, 
		int to_num_node, workload_feature feat){
	if(est_map.find(vm_id) == est_map.end() 
			|| est_map[vm_id].find({from_num_node, to_num_node}) == est_map[vm_id].end()){ 
		return default_topo_change_estimate_ms(vm_id, from_num_node, to_num_node);
	}
	int max_similar_score = -1;
	int cost_estimation = -1;
	for(auto& x: est_map[vm_id][{from_num_node, to_num_node}]){
		int similar_score = feat.similarity(x.first);
		if(similar_score > max_similar_score){
			similar_score = max_similar_score;
			cost_estimation = x.second;
		}
	}
	if(max_similar_score == -1)
		return default_topo_change_estimate_ms(vm_id, from_num_node, to_num_node);
	return cost_estimation;
}
