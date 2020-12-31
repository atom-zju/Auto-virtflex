#include "topo_change_engine.h"
#include <iostream>
#include <cassert>

static int  average_bw_changeness(vm* v){
	static bool toggle = true;
	toggle = !toggle;
	if(toggle)
		return 1;
	else 
		return -1;
	//return 0;
}
static int lowest_bw_candidate(vm* v, int num, vector<int>& can){
	can.push_back(3);
	return 0;
}
static int first_available_candidate(vm* v, int num, vector<int>& can){
	can.push_back(3);
	return 0;
}

void topo_change_engine::config(){
	topo_changeness = &average_bw_changeness;
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

