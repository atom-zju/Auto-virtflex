#ifndef TOPO_SYS_MAP_GENERATOR_H
#define TOPO_SYS_MAP_GENERATOR_H

#include "topo_change_engine.h"
#include "sys_map.h"
#include "topo_change_policy.h"
#include <unordered_map>

extern unordered_map<topo_change_policy, int(*)(topo_change_engine*, sys_map<int>&, topo_change_policy)> policy_func_map;

class topo_sys_map_generator{
	private:
		topo_change_engine* topo_ce;
	public:
		topo_sys_map_generator(topo_change_engine* topo_ce);
		int generate_topo_sys_map(sys_map<int>& new_sys, topo_change_policy topo_cp);
};

#endif
