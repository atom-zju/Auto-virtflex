#ifndef CONFLICT_RULE_H 
#define CONFLICT_RULE_H

#include<string>

using namespace std;

// priority classes: 
// 	0 : highest priority
//	10 : default priority if not specified

class conflict_rule{
	private:
		int priority;
		string name;
		double (*cal_score_func)(int vm_id, int pnode_id);
	public:
		conflict_rule(string name,int priority = 10): priority(priority), 
			name(name), cal_score(NULL){}
		void assign_cal_score_func(double (*cal_score_func)(int vm_id, int pnode_id));
		double cal_score(int vm_id, int pnode_id);
};
#endif
