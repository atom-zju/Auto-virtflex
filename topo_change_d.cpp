#include "topo_change_d.h"

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
