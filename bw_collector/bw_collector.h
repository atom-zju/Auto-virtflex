#ifndef BW_COLLECTOR_H
#define BW_COLLECTOR_H

#include <sys/time.h>
#include <vector>
//#include "topo_change_daemon.h"

using namespace std;

class bw_collector{
public: 
	int num_nodes;
	vector<int> pin_cpu_list;
	vector<int> cpu_msr_fd;
//	topo_change_d* topod;
	struct timeval prev_ts;
	struct timeval curr_ts;
	
	vector<long> config_msr_rd;
	vector<long> config_val_rd;
	vector<long> config_msr_wr;
	vector<long> config_val_wr;
	
	vector<long> value_msr_rd;
	vector<long> value_msr_wr;
	
	void init();
	void config_msr();
	void collect_val();
	void clear_msr();
	
//	bw_collector(topo_change_d* topod):topod(topod){};
	~bw_collector();
};

#endif
