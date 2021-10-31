#include "migration_cost_estimator.h"

int migration_cost_estimator::num_pages_to_be_migrated(sys_map<int>& old_sys, sys_map<int>& new_sys){
	// assuming no node sharing,
	// for current implementation, output is the number of nodes that has been changed instead of the
	// number of pages to be migrated
	int cnt = 0;
	for(auto& vm_id: old_sys.vm_list()){
		for(auto& vnode_id: old_sys.vnode_list(vm_id)){
			if(new_sys.record_exist_vm_view(vm_id, vnode_id) 
			&& new_sys.vm_view(vm_id, vnode_id).data != old_sys.vm_view(vm_id, vnode_id))
				cnt++;
		}
	}
	return cnt;
}
