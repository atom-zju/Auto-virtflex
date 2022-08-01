#ifndef AUTO_SCALE_RULE_TABLE_H
#define AUTO_SCALE_RULE_TABLE_H

#include "auto_scale_rule.h"
#include <vector>

using namespace std;

class auto_scale_rule_table{
	private:
		int vm_id;
		vector<auto_scale_rule> rule_table;
	public:
		auto_scale_rule_table(int vm_id): vm_id(vm_id){}
		void insert_rule(auto_scale_rule rule);
		void delete_rule(string rule_name);
		int calculate_scale_outcome(bool& topo_change); 
		// topo change indicate whether topo change is needed, if yes return 
		// the number of nodes that needs to be changed to
		~auto_scale_rule_table();
}
#endif

