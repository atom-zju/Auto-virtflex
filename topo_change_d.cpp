#include <iostream>
#include "topo_change_d.h"
#include "util.h"
using namespace std;

vm* topo_change_d::get_vm_by_id(int id){
	if(vm_map.find(id) != vm_map.end()){
		return vm_map[id];
	}
	return NULL;
}

topo_change_d::topo_change_d(){
	ts = 0;
	xs =  xs_daemon_open();
        if ( xs == NULL ) {
                perror("Error when connecting to a xs daemon\n");
                exit(-1);
        }
}

topo_change_d::~topo_change_d(){
	// free pointers in vm_map and pnode_list
	for(auto& x: vm_map){
		delete x.second;
	}

	for(auto& x: pnode_list){
		delete x;
	}
	xs_daemon_close(xs);

}

void topo_change_d::update_vm_map(){
	ts++;
	vector<string> dir;
	list_xenstore_directory(xs, string("/local/domain"), dir);
	cout<<"/local/domain"<< endl;
	for(auto& x: dir){
		int vm_id =  stoi(x);
		cout <<"\t" <<"VM: "<<stoi(x) << endl;
		if(vm_map.find(vm_id) == vm_map.end()){
			vm* v = new vm(vm_id, this, string("/local/domain/").append(x));
			v->update_vnode_map(ts);
			vm_map[vm_id] = v;	
		}
		else{
		// vm exists in vm_map
			vm_map[vm_id]->update_vnode_map(ts);
		}
	}

	// delete obsolete vm
	for(auto& x: vm_map){
		auto v = x.second;
		if(v->ts < ts){
			delete v;
			vm_map.erase(x.first);
		}
	}

}

int topo_change_d::shrink_vm(int id, int vnode_id){
	vm* v = get_vm_by_id(id);
	if(!v){
		cout<<"Didn't find vm " << id << " in topo_change_d::shrink_vm" <<endl;
		return -1;
	}
	return v->shrink_vnode(vnode_id);
}
int topo_change_d::expand_vm(int id, int vnode_id){
	vm* v = get_vm_by_id(id);
	if(!v){
		cout<<"Didn't find vm " << id << " in topo_change_d::expand_vm" <<endl;
		return -1;
	}
	return v->expand_vnode(vnode_id);
}
