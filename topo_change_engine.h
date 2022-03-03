#ifndef TOPO_CHANGE_EN_H
#define TOPO_CHANGE_EN_H

#include <vector>
#include <queue>
#include "vm.h"
#include "topo_change_d.h"
#include "workload_attr.h"
#include "topo_change_policy.h"
//#include "topo_sys_map_generator.h"

template<class T>
class sys_map;
class sys_map_base;
class topo_change_d;
class topo_sys_map_generator;
class runtime_estimator;
class migration_cost_estimator;
class performance_estimator;

class event_candidate{
public:
	int pri;
	vm* v;
	event_candidate(int p, vm* v):pri(p), v(v){}
};

struct comp {
     bool operator()(event_candidate& i, event_candidate& j) {
     return i.pri > j.pri;
    }
};

class topo_change_engine{
	friend int topo_changeness_sys_map_update(topo_change_d*, sys_map_base*, long long);
	friend int max_topo_change_net_gain_func(topo_change_engine*, sys_map<int>&, topo_change_policy);
	friend class runtime_estimator;
	friend class vm;
	private:
		priority_queue<event_candidate, vector<event_candidate>, comp> expand_heap;
		priority_queue<event_candidate, vector<event_candidate>, comp> shrink_heap;
		topo_change_d* topod;
		workload_attr* wlattr;
		unordered_map<string, sys_map_base*> sys_map_table;
		
		topo_sys_map_generator* topo_sys_map_gen;
		runtime_estimator* runtime_esti;
		performance_estimator* perf_esti;
		migration_cost_estimator* migration_cost_esti;

		int (*topo_changeness)(vm* v);
		int (*shrink_candidate)(vm* v, int num, vector<int>& can);
		int (*expand_candidate)(vm* v, int num, vector<int>& can);
		void calculate_topo_changeness();

		void print_last_record();
		void mark_sys_map_table_outdated();
		void generate_sys_map_table();
		int get_sys_topo(sys_map<int>& old_sys);
		bool can_preempt(int plus_vm_id, int minus_vm_id);
		int generate_new_topo_map(sys_map<int>& new_sys);
		/* generate_new_topo_map(&)
		 * 	policy_function_map[policy](&);
		 */
		int generate_topo_change_events(sys_map<int>& new_sys, sys_map<int>& old_sys,
						deque<topo_change_event>& e);

	public:
		topo_change_engine(topo_change_d* t);
		~topo_change_engine();
		void config();
		
		sys_map_base* get_sys_map(string sys_map_name); /////
		int generate_events(deque<topo_change_event>& e);
};

#endif
