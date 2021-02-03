#include "bw_collector.h"
#include <unistd.h>

int main(void){
	bw_collector bw;
	bw.init();
	bw.config_msr();
	while(1){
		bw.collect_val();
		usleep(1000000);
	}
	bw.clear_msr();
	return 0;
}
