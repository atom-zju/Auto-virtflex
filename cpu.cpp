#include "cpu.h"
#include "util.h"
#include "vm.h"
#include "topo_change_d.h"

void cpu::enable() const{
	write_to_xenstore_path(owner->topod->xs, owner->vcpu_path+"/"+to_string(cpuid)+"/availability", string("online"));
}

void cpu::disable() const{
	write_to_xenstore_path(owner->topod->xs, owner->vcpu_path+"/"+to_string(cpuid)+"/availability", string("offline"));

}
