#include "cpu.h"
#include "util.h"
#include "vm.h"
#include "topo_change_d.h"
#include <cassert>

void cpu::enable(){
	write_to_xenstore_path(owner->topod->xs, owner->vcpu_path+"/"+to_string(cpuid)+"/availability", string("online"));
	enabled = true;
}

void cpu::disable(){
	write_to_xenstore_path(owner->topod->xs, owner->vcpu_path+"/"+to_string(cpuid)+"/availability", string("offline"));
	enabled = false;
}

void cpu::update_usage(){
	assert(owner);
	assert(owner->topod);
	long long usg = xc_domain_get_cpu_usage(owner->topod->xc_handle, owner->vm_id, cpuid)/1000000;
	if( usg < 0 ){
		recent_usage_ms = 0;
		usage_ms = -1;
	}
	else{
		recent_usage_ms = usg - usage_ms;
		usage_ms = usg;
	}
}
