#include "topo_change_d.h"

int main(){
	topo_change_d topod;
	topod.set_interval_ms(1000);
	topod.start();
	//topod.update_vm_map();
	//topod.expand_vm(2, 3);	
	return 0;
}
