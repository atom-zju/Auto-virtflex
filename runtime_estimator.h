#ifndef RUNTIME_ESTIMATOR
#define RUNTIME_ESTIMATOR

#include "topo_change_engine.h"
#include "sample_queue.h"
#include <unordered_map>

class runtime_estimator{
	public:
		topo_change_engine* topo_ce;
		unordered_map<int, int> est_map; // maps vm_id to runtime in seconds. 
		
		runtime_estimator(topo_change_engine* topo_ce): topo_ce(topo_ce){};

		int remaining_runtime_in_sec(int vm_id, int curr_num_node);
		void update();
		
};


#endif
