#ifndef WORKLOAD_ATTR_H
#define WORKLOAD_ATTE_H

#include <unordered_map>

class topo_change_d;
struct xs_handle;

/* valid attr_name list:
 * 	MEM_IN_MB
 * 	THREAD
 * */

using namespace std;

class workload_attr{
	unordered_map<int, unordered_map<string, long long>> attr_map;
	struct xs_handle* xs;
	topo_change_d* topod;
	string xs_prefix;
public:
	workload_attr(struct xs_handle* xs, topo_change_d* topod);
	void update();
	void print();
	bool attr_exist(int vm_id, string attr_name);
	long long query_attr(int vm_id, string attr_name);
};

#endif
