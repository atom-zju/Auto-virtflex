#include "topo_change_d.h"

int main(){
	topo_change_d topod;
	topod.update_vm_map();
	topod.expand_vm(2, 3);	
	return 0;
}
