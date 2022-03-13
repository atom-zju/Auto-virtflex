#include "topo_change_status.h"
#include "workload_feature.h"
#include "vm.h"
#include "migration_cost_estimator.h"
#include <cassert>

topo_change_status::topo_change_status(vm* owner, migration_cost_estimator* esti){
	assert(owner);
	assert(esti);
	owner_vm = owner;
	migration_cost_esti = esti;
	status_type = topo_change_ready;
}

void topo_change_status::topo_change_status_change_begin(int from_num_node, int to_num_node){
	status_type = undergo_topo_change;
	assert(from_num_node != to_num_node);
	this->from_num_node = from_num_node;
	this->to_num_node = to_num_node;
	start_topo_change_ux_ts_ms = (long long)time(0)*1000;
	progress_finished_num_node = 0;
}

void topo_change_status::topo_change_finished_one_node(long long finished_ux_ts_ms){
	if( status_type != undergo_topo_change || finished_ux_ts_ms <= start_topo_change_ux_ts_ms)
		return;
	progress_finished_num_node++;
	if(progress_finished_num_node == abs(from_num_node - to_num_node)){
		// topo change ended
		insert_topo_change_cost_data(finished_ux_ts_ms);
		status_type = cool_down;
		cool_down_cd = TOPO_CHNAGENESS_TIME_WINDOW_SECS;
		progress_finished_num_node = 0;
	}
}

void topo_change_status::insert_topo_change_cost_data(long long finished_ux_ts_ms){
	end_topo_change_ux_ts_ms = finished_ux_ts_ms;
	if(!migration_cost_esti)
		return;
	assert(owner_vm);
	int cost_ms = finished_ux_ts_ms - start_topo_change_ux_ts_ms;
	assert(cost_ms >= 0);
	workload_feature wl_feat( owner_vm->vm_id, 
			start_topo_change_ux_ts_ms - 5000, start_topo_change_ux_ts_ms);
	migration_cost_esti->insert_topo_change_cost_esti_entry(
			owner_vm->vm_id, from_num_node, to_num_node, wl_feat,
			cost_ms);	
}

bool topo_change_status::is_undergo_topo_change(){
	return  status_type == undergo_topo_change || status_type == cool_down;
}

void topo_change_status::reduce_cool_down_cd(){
	if(status_type != cool_down)
		return;
	assert(cool_down_cd > 0);
	cool_down_cd--;
	if(cool_down_cd <= 0){
		status_type = topo_change_ready;
	}
}
