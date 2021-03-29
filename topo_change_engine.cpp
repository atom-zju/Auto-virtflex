#include "topo_change_engine.h"
#include <iostream>
#include <cassert>

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
	unsigned long low_thres = 500;
	unsigned long high_thres = 800;
	assert(v);
	long avg_bw = v->average_bw_usage();
	float avg_load = v->get_average_vcpu_load();
	cout << "TOPO_ENGINE: vm " << v->vm_id << ", avg_bw: " << avg_bw << endl;
	cout << "TOPO_ENGINE: vm " << v->vm_id << ", avg_load: " << avg_load << endl;
	if(avg_bw < 0 )
		return 0;
	if(avg_bw < low_thres && v->active_node > 1){
		cout << "\tShrink" << endl;
		return 0;
		//return -1;
	}
	if(avg_bw > high_thres && v->active_node < v->total_node){
		cout << "\tExpand" << endl;
		return 0;
		//return 1;
	}
	return 0;
}
static int  average_vcpu_load_changeness(vm* v){
	float low_thres = 0.1;
	float high_thres = 0.5;
	assert(v);
	long avg_bw = v->average_bw_usage();
	float avg_load = v->get_average_vcpu_load();
	cout << "TOPO_ENGINE: vm " << v->vm_id << ", avg_bw: " << avg_bw << endl;
	cout << "TOPO_ENGINE: vm " << v->vm_id << ", avg_load: " << avg_load << endl;
	if(avg_load < 0 )
		return 0;
	if(avg_load < low_thres && v->active_node > 1){
		cout << "\t------SHRINK-----" << endl;
		return -1;
	}
	if(avg_load > high_thres && v->active_node < v->total_node){
		cout << "\t++++++EXPAND+++++" << endl;
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
	
	vector<long> bw_usage;
	for(int i =0; i< can.size(); i++){
		auto node = v->get_vnode_by_id(can[i]);
		if(node){
			long bw = node->average_bw_usage();
			if(bw>=0){
				bw_usage.push_back(bw);
				continue;
			}
		}
		can.erase(can.begin()+i);
		i--;
	}
	
	if(num >= can.size()){
                return 0;
        }

	//get the lowest num elements in bw_usage and return the corresponding vnode id in can
	//can.push_back(3);
	priority_queue<pair<long, int>, vector<pair<long, int>>, comp_pair> max_h;
	for(int i=0; i < can.size(); i++){
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

void topo_change_engine::config(){
	topo_changeness = &average_bw_changeness;
	//topo_changeness = &average_vcpu_load_changeness;
	shrink_candidate = &lowest_bw_candidate;
	expand_candidate = &first_available_candidate; 

}

int topo_change_engine::generate_events(deque<topo_change_event>& e){
	assert(topo_changeness);
	assert(shrink_candidate);
	assert(e.empty());
	assert(topod);

	expand_heap = priority_queue <event_candidate, vector<event_candidate>, comp>();
	shrink_heap = priority_queue <event_candidate, vector<event_candidate>, comp>();
	
	// insert topo change candidate to heap according to topo_changeness
	for(auto& x: topod->vm_map){
		if (x.first == 0)
			continue;
		auto vm_ptr = x.second;
		int pir = (*topo_changeness)(vm_ptr);
		if(pir > 0){
			expand_heap.push(event_candidate(pir, vm_ptr));
		}
		else if(pir < 0){
			shrink_heap.push(event_candidate(-pir, vm_ptr));
		}
	}

	// process shrink events
	vector<int> nodes;
	while (!shrink_heap.empty()){
		nodes.clear();
		auto can = shrink_heap.top();
		shrink_heap.pop();
		if((*shrink_candidate)(can.v, 1, nodes) == 0)
			e.push_back(topo_change_event(can.v->vm_id, -1, nodes));
		else
			cout<< "Find shrink candidatefor vm " << can.v->vm_id << " failed" << endl;
	}

	//process expand events
	while (!expand_heap.empty()){
		nodes.clear();
		auto can = expand_heap.top();
		expand_heap.pop();
		if((*expand_candidate)(can.v, 1, nodes) == 0)
			e.push_back(topo_change_event(can.v->vm_id, 1, nodes));
		else
			cout<< "Find expand candidatefor vm " << can.v->vm_id << " failed" << endl;
	}
	
	return 0;
}

