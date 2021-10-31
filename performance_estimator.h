#ifndef PERFORMANCE_ESTIMATOR_H
#define PERFORMANCE_ESTIMATOR_H

#include "topo_change_engine.h"

class performance_estimator{
	public:
		topo_change_engine* topo_ce;
		performance_estimator(topo_change_engine* topo_ce): topo_ce(topo_ce){};
		float performance_gain_after_change(int vm_id, int curr_num_node, int new_num_node);
};

#endif
