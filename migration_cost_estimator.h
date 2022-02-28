#ifndef MIGRATION_COST_ESTIMATOR_H
#define MIGRATION_COST_ESTIMATOR_H

#include "topo_change_engine.h"
#include "workload_feature.h"

struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const
    {
        auto hash1 = hash<T1>{}(p.first);
        auto hash2 = hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

class migration_cost_estimator{
	private:
		// vm_id, from_num_node, to_num_node --> vector <workload_feature, cost time in ms>
		unordered_map<int, unordered_map< pair<int, int>, 
			vector<pair<workload_feature, int>>, hash_pair
			>> est_map;
		float default_add_cost_per_node_ms;
		float default_remove_cost_per_node_ms;
		int default_topo_change_estimate_ms(int vm_id, int from_num_node,
				int to_num_node);
	public:
		topo_change_engine* topo_ce;
		migration_cost_estimator(topo_change_engine* topo_ce){
			default_add_cost_per_node_ms = 500;
			default_remove_cost_per_node_ms = 1000;
		};
		int num_pages_to_be_migrated(sys_map<int>& old_sys, sys_map<int>& new_sys);
		int topo_change_cost_estimate_ms(int vm_id, int from_num_node, 
				int to_num_node, workload_feature feat);
};

#endif
