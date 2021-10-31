#ifndef MIGRATION_COST_ESTIMATOR_H
#define MIGRATION_COST_ESTIMATOR_H

#include "topo_change_engine.h"

class migration_cost_estimator{
	public:
		topo_change_engine* topo_ce;
		migration_cost_estimator(topo_change_engine* topo_ce){};
		int num_pages_to_be_migrated(sys_map<int>& old_sys, sys_map<int>& new_sys);
};

#endif
