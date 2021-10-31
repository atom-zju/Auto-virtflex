#include "topo_change_engine.h"
#include <iostream>
#include <cassert>
#include "sys_map.h"
#include "topo_sys_map_generator.h"
#include "runtime_estimator.h"
#include "performance_estimator.h"
#include "migration_cost_estimator.h"

/*       Topology changeness              */
static int  naive_toggle(vm* v){
	static bool toggle = true;
	toggle = !toggle;
	if(toggle)
		return 1;
	else 
		return -1;
}
static int  average_bw_changeness(vm* v){
	unsigned long low_thres = 150;
	unsigned long high_thres = 700;

	// test:
	//for (auto& x: sample_queue<int>::data_map[v->vm_id])
	//	if(x.first >= 0)
	//		for(auto& y: x.second)
	//			for(auto& z: y.second)
	//				z->print();
	

	assert(v);
	long avg_bw = v->average_bw_usage();
	float avg_load = v->get_average_vcpu_load();
	cout << UNIX_TS<< "TOPO_ENGINE: vm " << v->vm_id << ", avg_bw: " << avg_bw << endl;
	if(file_output)
	of << UNIX_TS<< "TOPO_ENGINE: vm " << v->vm_id << ", avg_bw: " << avg_bw << endl;
	cout << UNIX_TS<< "TOPO_ENGINE: vm " << v->vm_id << ", avg_load: " << avg_load << endl;
	if(file_output)
	of << UNIX_TS<< "TOPO_ENGINE: vm " << v->vm_id << ", avg_load: " << avg_load << endl;
	if(avg_bw < 0 )
		return 0;
	if(avg_bw < low_thres && v->active_node > 1){
		cout << UNIX_TS<< "\tShrink" << endl;
		if(file_output)
		of << UNIX_TS<< "\tShrink" << endl;
		//return 0;
		return -1;
	}
	if(avg_bw > high_thres && v->active_node < v->total_node){
		cout << UNIX_TS<< "\tExpand" << endl;
		if(file_output)
		of << UNIX_TS<< "\tExpand" << endl;
		//return 0;
		return 1;
	}
	return 0;
}
static int  average_vcpu_load_changeness(vm* v){
	float low_thres = 0.1;
	float high_thres = 0.5;
	assert(v);
	long avg_bw = v->average_bw_usage();
	float avg_load = v->get_average_vcpu_load();
	cout << UNIX_TS<< "TOPO_ENGINE: vm " << v->vm_id << ", avg_bw: " << avg_bw << endl;
	if(file_output)
	of << UNIX_TS<< "TOPO_ENGINE: vm " << v->vm_id << ", avg_bw: " << avg_bw << endl;
	cout << UNIX_TS<< "TOPO_ENGINE: vm " << v->vm_id << ", avg_load: " << avg_load << endl;
	if(file_output)
	of << UNIX_TS<< "TOPO_ENGINE: vm " << v->vm_id << ", avg_load: " << avg_load << endl;
	if(avg_load < 0 )
		return 0;
	if(avg_load < low_thres && v->active_node > 1){
		cout << UNIX_TS<< "\t------SHRINK-----" << endl;
		if(file_output)
		of << UNIX_TS<< "\t------SHRINK-----" << endl;
		return -1;
	}
	if(avg_load > high_thres && v->active_node < v->total_node){
		cout << UNIX_TS<< "\t++++++EXPAND+++++" << endl;
		if(file_output)
		of << UNIX_TS<< "\t++++++EXPAND+++++" << endl;
		return 1;
	}
	return 0;
}

struct comp_pair{
	bool operator()(pair<long, int>& l, pair<long, int>& r){
		return l.first <= r.first;
	}
};

/*       Shrink candidate              */

static int lowest_bw_candidate(vm* v, int num, vector<int>& can){
	v->active_node_list(can);

	if(num >= can.size()){
		return 0;
	}
	
	vector<long> bw_usage(can.size(), -1);
	for(int i =0; i< can.size(); i++){
		auto node = v->get_vnode_by_id(can[i]);
		if(node){
			long bw = node->average_bw_usage();
			if(bw>=0){
				bw_usage[i] = bw;
				//continue;
			}
		}
		//can.erase(can.begin()+i);
		//i--;
	}
	
	//if(num >= can.size()){
        //        return 0;
        //}

	//get the lowest num elements in bw_usage and return the corresponding vnode id in can
	//can.push_back(3);
	priority_queue<pair<long, int>, vector<pair<long, int>>, comp_pair> max_h;
	for(int i=0; i < can.size(); i++){
		// skip node 0 and nodes that don't have valid bw usage
		if(can[i] == 0 || bw_usage[i] == -1)
			continue;
		max_h.push(make_pair(bw_usage[i], can[i]));
		if(max_h.size() > num){
			max_h.pop();
		}
	}
	can.clear();
	while(!max_h.empty()){
		can.push_back(max_h.top().second);
		max_h.pop();
	}

	return 0;
}

/*       Expand candidate              */
static int first_available_candidate(vm* v, int num, vector<int>& can){
	v->inactive_node_list(can);
	
	if(num >= can.size()){
		return 0;
	}
	
	vector<long> active_competitor;
	for(int i =0; i< can.size(); i++){
		auto node = v->get_vnode_by_id(can[i]);
		if(node){
			long cnt = node->active_nodes_in_pnode();
			active_competitor.push_back(cnt);
			continue;
		}
		can.erase(can.begin()+i);
		i--;
	}
	
	if(num >= can.size()){
		return 0;
	}
	
	//get the lowest num elements in active_competitor and return the corresponding vnode id in can
	//can.push_back(3);
	priority_queue<pair<long, int>, vector<pair<long, int>>, comp_pair> max_h;
	for(int i=0; i < can.size(); i++){
		max_h.push(make_pair(active_competitor[i], can[i]));
		if(max_h.size() > num){
			max_h.pop();
		}
	}
	can.clear();
	while(!max_h.empty()){
		can.push_back(max_h.top().second);
		max_h.pop();
	}

	return 0;
}

topo_change_engine::topo_change_engine(topo_change_d* t): topod(t){
	wlattr = new workload_attr(topod->xs, topod);
	topo_sys_map_gen = new topo_sys_map_generator(this);
	runtime_esti = new runtime_estimator(this);
	perf_esti = new performance_estimator(this);
	migration_cost_esti = new migration_cost_estimator(this);
}

topo_change_engine::~topo_change_engine(){
	if(wlattr)
		delete wlattr;
	for(auto& x: sys_map_table)
		if(x.second)
			delete x.second;
	if(topo_sys_map_gen)
		delete topo_sys_map_gen;
	if(runtime_esti)
		delete runtime_esti;
	if(perf_esti)
		delete perf_esti;
	if(migration_cost_esti)
		delete migration_cost_esti;
}

void topo_change_engine::config(){
	topo_changeness = &average_bw_changeness;
	//topo_changeness = &average_vcpu_load_changeness;
	shrink_candidate = &lowest_bw_candidate;
	expand_candidate = &first_available_candidate; 

}

void topo_change_engine::calculate_topo_changeness(){
	// testing sys_map class	
	//sys_map<int> topo_map(TOPO_SYS_MAP);
	//topo_map.update(topod);
	//topo_map.print();
	//sys_map<float> vcpu_usage_map(VCPU_USAGE_SYS_MAP);
	//vcpu_usage_map.update(topod, (time(NULL)-1)*1000);
	//vcpu_usage_map.print();
	//sys_map<int> bw_usage_map(BW_USAGE_SYS_MAP);
	//bw_usage_map.update(topod, (time(NULL)-5)*1000);
	//bw_usage_map.print();
	//float r = vcpu_usage_map.vm_view(1, 3).data;
	//cout << "vm_view(1, 3): " << (vcpu_usage_map.vm_view(1, 3).is_empty()? -2333.0: r) << endl;

	for(auto& x: topod->vm_map){
		if(x.first == 0)
			continue;
		auto vm_ptr = x.second;
		vm_ptr->calculate_topo_changeness();
		cout<< UNIX_TS<< "TOPO_CHANGENESS vm " << vm_ptr->vm_id << ":" << vm_ptr->topo_changeness << endl;
		if(file_output)
		of<< UNIX_TS<< "TOPO_CHANGENESS vm " << vm_ptr->vm_id << ":" << vm_ptr->topo_changeness << endl;
		
	}
}

int topo_change_engine::get_sys_topo(sys_map<int>& old_sys){
	if(sys_map_table.find(TOPO_SYS_MAP) == sys_map_table.end())
		return -1;	
	old_sys = *(sys_map<int>*)sys_map_table[TOPO_SYS_MAP];
	if(old_sys.ux_ts_ms < (time(NULL)-5)*1000)
		old_sys.update(topod);
	return 0;
}


void topo_change_engine::print_last_record(){
	sys_map_base* topo_sys_ptr = get_sys_map(TOPO_SYS_MAP);
	sys_map_base* bw_sys_ptr = get_sys_map(BW_USAGE_SYS_MAP);
	sys_map_base* cpu_sys_ptr = get_sys_map(VCPU_USAGE_SYS_MAP);

	assert(topo_sys_ptr && topo_sys_ptr->get_base_type() == "int");
	sys_map<int> topo_sys = *(sys_map<int>*)topo_sys_ptr;
	assert(bw_sys_ptr && bw_sys_ptr->get_base_type() == "int");
	sys_map<int> bw_sys = *(sys_map<int>*)bw_sys_ptr;
	assert(cpu_sys_ptr && cpu_sys_ptr->get_base_type() == "float");
	sys_map<float> cpu_sys = *(sys_map<float>*)cpu_sys_ptr;

	topo_sys.update(topod, -1);
	bw_sys.update(topod, -1);
	cpu_sys.update(topod, -1);

	bw_sys.prune(topo_sys);
	cpu_sys.prune(topo_sys);
	
	for(auto& vm_id: bw_sys.vm_list()){
		of << UNIX_TS << "vm: " << vm_id << ", avg_bw_load: " << bw_sys.vm_avg(vm_id) << endl;	
	}
	for(auto& vm_id: cpu_sys.vm_list()){
		of << UNIX_TS <<  "vm: " << vm_id << ", avg_cpu_load: "<<cpu_sys.vm_avg(vm_id) << endl;
	}	

}
/* checks whether home node sys (can be half-completed) is valid */
static bool home_node_sys_is_valid(sys_map<int>& home_node_sys, sys_map<int>& topo_sys){
	bool idle = false;
	bool overlap = false;
	for(auto& pnode_id: home_node_sys.pnode_list()){
		int pnode_sum = home_node_sys.pnode_sum(pnode_id);
		if(pnode_sum == 0)
			idle = true;
		else if(pnode_sum > 1)
			overlap = true;
		if(idle && overlap)
			return false;
	}
	// checks more than one home node per vm
	for(auto& vm_id: home_node_sys.vm_list())
		if(home_node_sys.vm_sum(vm_id) > 1)
			return false;
	// checks home node are enabled node
	for(auto& vm_id: home_node_sys.vm_list())
		for(auto& vnode_id: home_node_sys.vnode_list(vm_id))
			if(home_node_sys.vm_view(vm_id, vnode_id).data == 1 
					&& topo_sys.vm_view(vm_id, vnode_id).data != 1)
				return false;
	return true;
}
/* check whether every vm is assigned to a home node */
static int home_node_sys_is_complete(sys_map<int>& home_node_sys, sys_map<int>& topo_sys){
	for(auto& vm_id: topo_sys.vm_list())
		if(home_node_sys.vm_sum(vm_id) < 1)
			return false;
	return true;
}

static bool home_node_assignment_helper(sys_map<int>& topo_sys, 
		const vector<int>& vm_list, int idx, sys_map<int>& home_node_sys){
	if(idx >= vm_list.size())
		return true;
	int vm_id = vm_list[idx];
	for(auto& vnode_id: topo_sys.vnode_list(vm_id))
		if(topo_sys.vm_view(vm_id, vnode_id).data == 1){
			int pnode_id = topo_sys.vm_view(vm_id, vnode_id).pnode_id;
			if(home_node_sys.pnode_sum(pnode_id) > 0){
				bool skip = false;
				for(auto& pnode_id: home_node_sys.pnode_list())
					if(home_node_sys.pnode_sum(pnode_id) == 0){
						skip = true;
						break;
					}
				if (skip)
					continue;
			}
			home_node_sys.vm_view(vm_id, vnode_id).data = 1;
			if(home_node_assignment_helper(topo_sys, vm_list, idx+1, home_node_sys))
				return true;
			else
				home_node_sys.vm_view(vm_id, vnode_id).data = 0;
		}
	return false;
}

static bool home_node_assignment(topo_change_d* topod, sys_map<int> topo_sys, sys_map<int> home_node_sys){
	if(home_node_sys_is_valid(home_node_sys, topo_sys) 
			&& home_node_sys_is_complete(home_node_sys, topo_sys))
		return false;
	//sys_map<int> old_home_node_sys = home_node_sys;
	home_node_sys.clear();
	home_node_sys.same_dimension_zero_fill(topo_sys);
	if(!home_node_assignment_helper(topo_sys, topo_sys.vm_list(), 0, home_node_sys))
		return false;
	for(auto& vm_id: home_node_sys.vm_list()){
		int vnode_id = home_node_sys.max_vnode_in_vm(vm_id);
		assert(home_node_sys.vm_view(vm_id, vnode_id).data == 1);
		topod->set_reserved_vnode_id(vm_id, vnode_id);
	}
	return true;
}

void topo_change_engine::generate_sys_map_table(){
	if(sys_map_table.find(TOPO_SYS_MAP) == sys_map_table.end())
		sys_map_table[TOPO_SYS_MAP] = new sys_map<int>(TOPO_SYS_MAP, this);
	if(sys_map_table.find(VCPU_USAGE_SYS_MAP) == sys_map_table.end())
                sys_map_table[VCPU_USAGE_SYS_MAP] = new sys_map<float>(VCPU_USAGE_SYS_MAP, this);
        if(sys_map_table.find(BW_USAGE_SYS_MAP) == sys_map_table.end())
                sys_map_table[BW_USAGE_SYS_MAP] = new sys_map<int>(BW_USAGE_SYS_MAP, this);
        if(sys_map_table.find(HOME_NODE_SYS_MAP) == sys_map_table.end())
                sys_map_table[HOME_NODE_SYS_MAP] = new sys_map<int>(HOME_NODE_SYS_MAP, this);
        if(sys_map_table.find(NUM_THREAD_SYS_MAP) == sys_map_table.end())
                sys_map_table[NUM_THREAD_SYS_MAP] = new sys_map<int>(NUM_THREAD_SYS_MAP, this);
        if(sys_map_table.find(NUM_VCPU_SYS_MAP) == sys_map_table.end())
                sys_map_table[NUM_VCPU_SYS_MAP] = new sys_map<int>(NUM_VCPU_SYS_MAP, this);
        if(sys_map_table.find(NODE_SIZE_SYS_MAP) == sys_map_table.end())
                sys_map_table[NODE_SIZE_SYS_MAP] = new sys_map<int>(NODE_SIZE_SYS_MAP, this);
}

sys_map_base* topo_change_engine::get_sys_map(string sys_map_name){
	if(sys_map_table.find(sys_map_name) == sys_map_table.end()){
		if(sys_map_info_map.find(sys_map_name) == sys_map_info_map.end())
			return NULL;
		if(sys_map_info_map[sys_map_name].second == "int")
			sys_map_table[sys_map_name] = new sys_map<int>(sys_map_name, this); /////
		else if(sys_map_info_map[sys_map_name].second == "float")
			sys_map_table[sys_map_name] = new sys_map<float>(sys_map_name, this);
		else
			return NULL;
	}
	if(sys_map_table[sys_map_name]->is_outdated()){
		sys_map_table[sys_map_name]->update(topod, 10);
		sys_map_table[sys_map_name]->print();
	}
	return sys_map_table[sys_map_name];
}


bool topo_change_engine::can_preempt(int plus_vm_id, int minus_vm_id){
	auto topo_changeness_sys = (sys_map<int>*)get_sys_map(TOPO_CHANGENESS_SYS_MAP);
	int plus_vm_score = topo_changeness_sys->vm_sum(plus_vm_id);
	int minus_vm_score = topo_changeness_sys->vm_sum(minus_vm_id);
	if(plus_vm_score > 50 && plus_vm_score > 2*minus_vm_score)
		return true;
	return false;
}

int topo_change_engine::generate_new_topo_map(sys_map<int>& new_sys){
	
	auto topo_sys = (sys_map<int>*)get_sys_map(TOPO_SYS_MAP);
	auto home_node_sys = (sys_map<int>*)get_sys_map(HOME_NODE_SYS_MAP);
	home_node_assignment(topod, *topo_sys, *home_node_sys);
	home_node_sys->update(topod);

	assert(topo_sys_map_gen);
	return topo_sys_map_gen->generate_topo_sys_map(new_sys, max_topo_change_net_gain);
	
//	auto topo_changeness_sys = (sys_map<int>*)get_sys_map(TOPO_CHANGENESS_SYS_MAP);
//	auto shrink_node_rank_sys = (sys_map<int>*)get_sys_map(SHRINK_NODE_RANK_SYS_MAP);
//	new_sys = *topo_sys;
//
//
//	vector<int> vm_list;
//	vector<int> topo_changeness_list;
//	queue<int> free_pnode_q;
//	int hi;
//	topo_changeness_sys->sort_vm_by_sum_ascend(vm_list);
//	for(auto& pnode_id: new_sys.pnode_list()){
//		if(new_sys.pnode_sum(pnode_id) == 0)
//			free_pnode_q.push(pnode_id);
//	}
//	for(auto& vm_id: vm_list){
//		topo_changeness_list.push_back(topo_changeness_sys->vm_sum(vm_id));
//	}
//	// phase 1: natural trade, plus vm take away resources from minus vm
//	// processing shrinking
//	for(auto& vm_id: vm_list){
//		if(topo_changeness_sys->vm_sum(vm_id) >= 0){
//			break;
//		}
//		int vnode_id = shrink_node_rank_sys->max_vnode_in_vm(vm_id);
//		shrink_node_rank_sys->vm_view(vm_id, vnode_id).data = -200;
//		new_sys.vm_view(vm_id, vnode_id).data = 0;
//		free_pnode_q.push(new_sys.vm_view(vm_id, vnode_id).pnode_id);
//		cout << "shrinking vm " << vm_id << " vnode:" << vnode_id << endl;
//	}
//	// processing expansion
//	for(hi=vm_list.size()-1; hi >= 0; hi--){
//		cout << "considering vm_id :" << vm_list[hi] << endl;
//		if(free_pnode_q.empty())
//			break;
//		if(topo_changeness_sys->vm_sum(vm_list[hi]) <= 0 )
//			return 0;
//		auto pnode_id = free_pnode_q.front();
//		free_pnode_q.pop();
//		int vnode_id = topod->pnode_to_vnode(vm_list[hi], pnode_id);
//		new_sys.vm_view(vm_list[hi], vnode_id).data = 1;
//		cout << "expanding vm " << vm_list[hi] << " vnode:" << vnode_id << endl;
//	}
//	// phase 2: extra trade (include preempt)
//	int lo=0;
//	while(lo < hi){
//		if(topo_changeness_list[hi] <= 0)
//			return 0;
//		if(new_sys.vm_sum(vm_list[lo]) > 1){
//			if(topo_changeness_list[lo] >= 0 && !can_preempt(vm_list[hi], vm_list[lo]))
//				return 0;
//			int lo_vnode_id = shrink_node_rank_sys->max_vnode_in_vm(vm_list[lo]);
//                	new_sys.vm_view(vm_list[lo], lo_vnode_id).data = 0;	
//                	shrink_node_rank_sys->vm_view(vm_list[hi], lo_vnode_id).data = -200;
//			cout << "shrinking vm " << vm_list[lo] << " vnode:" << lo_vnode_id << endl;
//			int hi_vnode_id = topod->pnode_to_vnode(vm_list[hi], 
//					topod->vnode_to_pnode(vm_list[lo], lo_vnode_id));
//                	new_sys.vm_view(vm_list[hi], hi_vnode_id).data = 1;
//			cout << "expanding vm " << vm_list[hi] << " vnode:" << hi_vnode_id << endl;
//			hi--;
//		}
//		else{
//			lo++;
//		}
//	}
//	return 0;	
}

int topo_change_engine::generate_topo_change_events(sys_map<int>& new_sys, sys_map<int>& old_sys,
                                                deque<topo_change_event>& e){
	for(auto& vm_id: old_sys.vm_list())
		for(auto& vnode_id: old_sys.vnode_list(vm_id)){
			auto old_data = old_sys.vm_view(vm_id,vnode_id);
			auto new_data = new_sys.vm_view(vm_id,vnode_id);
			assert(!old_data.is_empty() && !new_data.is_empty());
			// shrink
			if(old_data.data == 1 && new_data.data == 0){
				e.insert(e.begin(), topo_change_event(vm_id, -1, vector<int>({vnode_id})));
			}
			// expand
			else if(old_data.data == 0 && new_data.data == 1){
				e.push_back(topo_change_event(vm_id, 1, vector<int>({vnode_id})));
			}
		}
	return 0;	
}

void topo_change_engine::mark_sys_map_table_outdated(){
	for(auto& x: sys_map_table)
		x.second->mark_outdated();
}

int topo_change_engine::generate_events(deque<topo_change_event>& e){
	generate_sys_map_table();
	print_last_record();
	sys_map<int> old_sys, new_sys;
	wlattr->update();
	wlattr->print();
	if(generate_new_topo_map(new_sys)< 0)
		return -1;
	if(get_sys_topo(old_sys) <0)
		return -1;
	if(generate_topo_change_events(new_sys, old_sys, e) < 0)
		return -1;
	mark_sys_map_table_outdated();
	return 0;
}

