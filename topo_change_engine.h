#ifndef TOPO_CHANGE_EN_H
#define TOPO_CHANGE_EN_H

#include <vector>
#include <queue>
#include "vm.h"
#include "topo_change_d.h"

class topo_change_d;

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
	private:
		priority_queue<event_candidate, vector<event_candidate>, comp> expand_heap;
		priority_queue<event_candidate, vector<event_candidate>, comp> shrink_heap;
		topo_change_d* topod;
		int (*topo_changeness)(vm* v);
		int (*shrink_candidate)(vm* v, int num, vector<int>& can);
		int (*expand_candidate)(vm* v, int num, vector<int>& can);
	public:
		topo_change_engine(topo_change_d* t): topod(t) {}	
		void config();
		int generate_events(deque<topo_change_event>& e);
};

#endif
