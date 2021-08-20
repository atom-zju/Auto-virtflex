#include "workload_attr.h"
#include "topo_change_d.h"
#include "util.h"

using namespace std;

workload_attr::workload_attr(struct xs_handle* xs, topo_change_d* topod): topod(topod), xs(xs){
	xs_prefix = topod->get_xs_path();
}

void workload_attr::print(){
	cout << "workload_attr content:" << endl;
	for(auto& vm: attr_map){
		cout << "    vm:" << vm.first << endl;
		for(auto& attr: attr_map[vm.first])
			cout << "        attr_name: " << attr.first << ", attr_val: " << attr.second << endl;
	}
}

void workload_attr::update(){
	attr_map.clear();
	for(auto& vm_id: topod->vm_list()){
		string xs_path = xs_prefix+"/"+to_string(vm_id)+"/numa/workload";
		if(get_workload_attr_from_xs(xs, xs_path, attr_map[vm_id]) < 0) /////
			cout << "no workload attr for vm: " << vm_id << endl; 
	}
}

bool workload_attr::attr_exist(int vm_id, string attr_name){
	if(attr_map.find(vm_id) == attr_map.end() || 
			attr_map[vm_id].find(attr_name) == attr_map[vm_id].end())
		return false;
	return true;
}

long long workload_attr::query_attr(int vm_id, string attr_name){
	assert(attr_map.find(vm_id) != attr_map.end() &&
                        attr_map[vm_id].find(attr_name) != attr_map[vm_id].end());
	return attr_map[vm_id][attr_name];
}
