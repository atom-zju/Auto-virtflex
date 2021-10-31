#ifndef RUNTIME_ESTIMATOR
#define RUNTIME_ESTIMATOR

#include "topo_change_engine.h"

class runtime_estimator{
	public:
		topo_change_engine* topo_ce;
		runtime_estimator(topo_change_engine* topo_ce): topo_ce(topo_ce){};
		int remaining_runtime_in_sec(int vm_id);
};


#endif
