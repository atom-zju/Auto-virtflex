#include "vnode.h"
#include "topo_change_d.h"
#include <iostream>

void vnode::update_node(unsigned int ts){
	this->ts = ts;
	// update capacity and cpu list
	string tmp;
	vector<string> tmp_list;

	read_from_xenstore_path(topod->xs, string(xs_path).append("/capacity"), tmp);
	capacity = stoi(tmp);
	cout << "capacity: " << capacity <<endl;

	// be carefull not to insert all the vcpus one more time everytime you update node
	list_xenstore_directory(topod->xs, string(xs_path).append("/vcpu"),tmp_list);
	for(auto& x: tmp_list){
		cout<< "vcpu: " << x <<endl;
		cpu_set.insert(cpu(stoi(x),true));
	}
	
}
