#include "cpu.h"
#include "util.h"
#include "vm.h"
#include "topo_change_d.h"
#include <cassert>

void cpu::init(struct timeval* now){
	assert(owner);
	ts = *now;
	usage_ms = xc_domain_get_cpu_usage(owner->topod->xc_handle, owner->vm_id, cpuid)/1000000;
}

void cpu::enable(){
	write_to_xenstore_path(owner->topod->xs, owner->vcpu_path+"/"+to_string(cpuid)+"/availability", string("online"));
	enabled = true;
}

void cpu::disable(){
	write_to_xenstore_path(owner->topod->xs, owner->vcpu_path+"/"+to_string(cpuid)+"/availability", string("offline"));
	enabled = false;
}

void cpu::update_usage(struct timeval* now){
	assert(owner);
	assert(owner->topod);
	long long time_elapsed_ms = (now->tv_sec-ts.tv_sec)*1000 + (now->tv_usec-ts.tv_usec)/1000;
	ts = *now;
	long long ux_ts_ms = now->tv_sec*1000 + now->tv_usec/1000;
	long long usg = xc_domain_get_cpu_usage(owner->topod->xc_handle, owner->vm_id, cpuid)/1000000;
	if( usg < 0 ){
		recent_usage_ms = 0;
		usage_ms = -1;
		recent_usage_percent = 0;
	}
	else{
		recent_usage_ms = usg - usage_ms;
		usage_ms = usg;
		recent_usage_percent = time_elapsed_ms? (float)recent_usage_ms/(float)time_elapsed_ms: 0;
		samples.push_back(make_pair(ux_ts_ms, recent_usage_percent));
		if(samples.size()> MAX_LOCAL_CPU_SP_SIZE)
			samples.pop_front();
	}
}
