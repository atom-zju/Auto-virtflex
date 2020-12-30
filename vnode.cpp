#include "vnode.h"
#include "topo_change_d.h"
#include <iostream>
#include <cassert>

void vnode::update_node(unsigned int ts){
	this->ts = ts;
	// update capacity and cpu list
	string tmp;
	vector<string> tmp_list;

	assert(topod);
	read_from_xenstore_path(topod->xs, string(xs_path).append("/capacity"), tmp);
	capacity = stoi(tmp);
	cout << "capacity: " << capacity <<endl;

	read_from_xenstore_path(topod->xs, string(xs_path).append("/target"), tmp);
	target = stoi(tmp);
	cout << "target: " << target <<endl;

	// calculate low target base on capacity, formula:
	// low_target = (80*1024 + capacity/1024*17)
	low_target = (80*1024 + capacity/1024*17);
	cout << "low_target: " << low_target << endl;

	// be carefull not to insert all the vcpus one more time everytime you update node
	list_xenstore_directory(topod->xs, string(xs_path).append("/vcpu"),tmp_list);
	for(auto& x: tmp_list){
		cout<< "vcpu: " << x <<endl;
		cpu_set.insert(cpu(stoi(x),true, owner_vm));
	}
	
}

int vnode::shrink(){
	// 1. set the topo_change flag to be 2, path: /numa/topo_change
	// 2. change target to low_target 
	// 3. disable all vcpus
	string topo_change_flag(xs_path.substr(0, xs_path.find_last_of("/\\")));
	topo_change_flag = topo_change_flag.substr(0, topo_change_flag.find_last_of("/\\"));
	topo_change_flag.append("/topo_change");
	cout << "topo_change_flag path: " << topo_change_flag << endl;
	
	assert(topod);
	write_to_xenstore_path(topod->xs, topo_change_flag,string("2"));
	write_to_xenstore_path(topod->xs, string(xs_path).append("/target"),to_string(low_target));
	for(auto& x: cpu_set){
		x.disable();
	}
	
	return 0;	
}

int vnode::expand(){
	// 1. set the topo_change flag to be 1, path: /numa/topo_change
	// 2. change target to capacity
	// 3. enable all vcpus

	string topo_change_flag(xs_path.substr(0, xs_path.find_last_of("/\\")));
	topo_change_flag = topo_change_flag.substr(0, topo_change_flag.find_last_of("/\\"));
	topo_change_flag.append("/topo_change");
	cout << "topo_change_flag path: " << topo_change_flag << endl;
	
	assert(topod);
	write_to_xenstore_path(topod->xs, topo_change_flag,string("1"));
	write_to_xenstore_path(topod->xs, string(xs_path).append("/target"),to_string(capacity));
	for(auto& x: cpu_set){
		x.enable();
	}
	return 0;
}
