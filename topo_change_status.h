#ifndef TOPO_CHANGE_STATUS_H
#define TOPO_CHANGE_STATUS_H

class vm;
class migration_cost_estimator;

class topo_change_status{
private:
	vm* owner_vm;
	migration_cost_estimator* migration_cost_esti;
        
	bool undergo_topo_change;
	long long start_topo_change_ux_ts_ms;
	long long end_topo_change_ux_ts_ms;
        
	int from_num_node;
        int to_num_node;
        int progress_finished_num_node;

        void insert_topo_change_cost_data(long long finished_ux_ts_ms);
public:
	topo_change_status(vm* owner, migration_cost_estimator* esti);
	bool is_undergo_topo_change();
	
        void topo_change_status_change_begin(int from_num_node, int to_num_node);
        void topo_change_finished_one_node(long long finished_ux_ts_ms);
};

#endif

