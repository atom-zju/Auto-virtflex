#include <iostream>
#include "topo_change_d.h"
#include "util.h"
using namespace std;

topo_change_d::topo_change_d(){
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

void topo_change_d::init_vm_map(){
	vector<string> dir;
	list_xenstore_directory(xs, string("/local/domain"), dir);
	cout<<"/local/domain"<< endl;
	for(auto& x: dir){
		int vm_id =  stoi(x);
		cout <<"\t" <<"VM: "<<stoi(x) << endl;
		if(vm_map.find(vm_id) == vm_map.end()){
			vm* v = new vm(this, string("/local/domain/").append(x));
			v->init_vnode_map();
			vm_map[vm_id] = v;	
		}
	}
	
		

}
