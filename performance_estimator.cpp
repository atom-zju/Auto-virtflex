#include "performance_estimator.h"
#include <cassert>

float performance_estimator::performance_gain_after_change(int vm_id, int curr_num_node, int new_num_node){
	assert(curr_num_node);
	return (float)new_num_node/(float)curr_num_node;
}
