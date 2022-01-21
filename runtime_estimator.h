#ifndef RUNTIME_ESTIMATOR
#define RUNTIME_ESTIMATOR

//#include "topo_change_engine.h"
//#include "sample_queue.h"
#include <unordered_map>

class topo_change_engine;

using namespace std;

class runtime_estimator{
	public:
		topo_change_engine* topo_ce;
		unordered_map<int, pair<float, long long>> est_map; 
		// format: vm_id, pair<runtime, this workload start time(-1 means it's currently idle)>

		float recent_rt_weight; // used for exponential decay
		
		runtime_estimator(topo_change_engine* topo_ce): topo_ce(topo_ce), recent_rt_weight(0.5){};

		int remaining_runtime_in_sec(int vm_id, int curr_num_node);
		void update();
		void print();
		
};


#endif
