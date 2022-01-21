#include "runtime_estimator.h"
#include "topo_change_engine.h"
#include "topo_change_d.h"
#include "vm.h"
#include "util.h"

void runtime_estimator::print(){
	cout << "Runtime estimator content:" << endl;
	cout << "\t";
	for(auto& x: est_map){
		cout << "vm_id: " << x.first << ", avg_rt: " 
			<< x.second.first << ", last_start_time:" << x.second.second << " | ";
	}
	cout << endl;
}

int runtime_estimator::remaining_runtime_in_sec(int vm_id, int curr_num_node){
	// assumes that the runtime is 10s for all applications
	int ret = 10;
	if(est_map.find(vm_id) == est_map.end())
		return ret;
	pair<float, long long> p = est_map[vm_id];
	time_t elapsed_time = time(0)-p.second/1000;
	ret = p.first/1000/(float)curr_num_node - elapsed_time;
	return ret;
};

vector<pair<long long, int>> get_related_node_records(deque<pair<long long, int>>& nodeq, long long start_ts, long long end_ts){
	vector<pair<long long, int>> res;
	int start_idx = floor_idx_of_sampleq(nodeq, 0, nodeq.size()-1, start_ts);
	int end_idx = floor_idx_of_sampleq(nodeq, 0, nodeq.size()-1, end_ts);
	assert(start_idx>=0 &&  end_idx >=0);
	assert(start_idx < nodeq.size() && end_idx < nodeq.size());
	for(int i= start_idx; i <= end_idx; i++){
		res.push_back(nodeq[i]);
	}
	return res;
}

float crawl_runtime_since_ts_ux_ms(sample_queue<int>* idle_sq, 
		sample_queue<int>* active_node_sq, long long ts_ux_ms){
	assert(idle_sq);
	assert(active_node_sq);
	
	auto& idleq = idle_sq->sample;
	auto& nodeq = active_node_sq->sample;
	
	int i_start_idx = return_insert_index(idleq, 0, idleq.size()-1, ts_ux_ms, NULL);

	float sum = 0;
	int cnt = 0;
	for(int i = i_start_idx; i < idleq.size(); i++){
		while(i < idleq.size() && idleq[i].second == 0) i++;
		if(i >= idleq.size())
			break;
		assert(i+1 < idleq.size() && idleq[i+1].second == 0);
		long long start_ts = idleq[i].first;
		long long end_ts = idleq[i+1].first;
		vector<pair<long long, int>> related_node_records = get_related_node_records(nodeq, start_ts, end_ts);
		int j;
		for(j=1; j < related_node_records.size(); j++){
			long long ets = min(related_node_records[j].first, end_ts);
			long long sts = max(related_node_records[j-1].first, start_ts);
			sum += (ets-sts)*related_node_records[j-1].second;
		}
		long long ets = end_ts;
		long long sts = max(related_node_records[j-1].first, start_ts);
		sum += (ets-sts)*related_node_records[j-1].second;
		cnt++;
		i++;
	}
	return cnt ==0? 0:sum/cnt;
}

//unordered_map<int, pair<float, long long>> est_map;
//format: vm_id, pair<runtime, this workload start time(-1 means it's currently idle)>

void runtime_estimator::update(){
	// iterate through all of the vms
	auto& sq_map = sample_queue<int>::data_map;
	for(pair<const int, vm*>& p: topo_ce->topod->vm_map){
		int vm_id = p.first;
		vm* vm_ptr = p.second;

		assert(sq_map[vm_id][SYS_NODE_ID].find(IDLENESS_SQ) != sq_map[vm_id][SYS_NODE_ID].end());
		assert(!sq_map[vm_id][SYS_NODE_ID][IDLENESS_SQ].empty());
		assert(sq_map[vm_id][SYS_NODE_ID].find(NUM_ACTIVE_NODE_SQ) != sq_map[vm_id][SYS_NODE_ID].end());
		assert(!sq_map[vm_id][SYS_NODE_ID][NUM_ACTIVE_NODE_SQ].empty());

		auto idle_sq = sq_map[vm_id][SYS_NODE_ID][IDLENESS_SQ][0];
		auto active_node_sq = sq_map[vm_id][SYS_NODE_ID][NUM_ACTIVE_NODE_SQ][0];

		if(est_map.find(vm_id) == est_map.end()){
			// if est entry not found, crawl sq to create entry, only update when is not running
			//if(!vm_ptr->is_running_workload()){
			if(!idle_sq->sample.empty() && idle_sq->sample.back().second == 0){
				float rt = crawl_runtime_since_ts_ux_ms(idle_sq, active_node_sq, 0); 
				est_map[vm_id] = make_pair(rt, -1);
			}
		}
		else if(!idle_sq->sample.empty() &&idle_sq->sample.back().second == 1//means last idle record is 1 
				&& est_map[vm_id].second == -1){
			// if just started a workload, record start time
			//est_map[vm_id].second = time(0)*1000;
			est_map[vm_id].second = idle_sq->sample.back().first;
		}	
		else if(!idle_sq->sample.empty()&&idle_sq->sample.back().second == 0 //means last idle record is 0
			       	&& est_map[vm_id].second != -1){
			// if just finished a workload, update est entry & strat time
			long long last_ts = est_map[vm_id].second;
			est_map[vm_id].second = -1;
			float rt = crawl_runtime_since_ts_ux_ms(idle_sq, active_node_sq, last_ts);
			est_map[vm_id].first = recent_rt_weight*rt + (1-recent_rt_weight)*est_map[vm_id].first;
		}
	}
}
