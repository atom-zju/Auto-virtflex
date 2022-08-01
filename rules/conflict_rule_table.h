#ifndef CONFLICT_RULE_TABLE_H
#define CONFLICT_RULE_TABLE_H

#include<vector>
#include "conflict_rule.h"

using namespace std;

class conflic_rule_table{
	private:
		vector<conflict_rule> rule_table;
	public:
		void insert_rule(conflict_rule rule);
		void delete_rule();
		void resolve_conflict();
};

#endif
