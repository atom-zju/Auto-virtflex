#ifndef AUTO_SCALE_RULE
#define AUTO_SCALE_RULE

#include<string>

// weight for auto scaling rules: higher -> more efficient/important
// 	default weight: 1

using namespace std;

enum rule_feedback{
	good,
	bad,
	neutral
};

class auto_scale_rule{
	private:
		int vm_id;
		// if activated == true, return number of nodes that should be set
		int (*rule_func)(int vm_id, bool& activated);  
		rule_feedback (*feedback_func)(int vm_id);
		string name;
		double weight;
	public:
		auto_scale_rule(string name, int weight, int vm_id): name(name), 
			weight(weight), vm_id(vm_id){}
		void assign_rule_func(int (*rule_func)(int vm_id, bool& activated));
		int check_rule(int vm_id, bool& activated);
		void assign_rule_fb_func(rule_feedback (*feedback_func)(int vm_id));
		void update_weight_w_fb(int vm_id);
};

#endif
