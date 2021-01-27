#ifndef CPU_H
#define CPU_H
#include <cstddef>
class vm;

class cpu{
public:
	int cpuid;
	bool enabled;
	vm* owner;
	long long usage_ms;
	long long recent_usage_ms;

	cpu(int id, bool en, vm* o): cpuid(id), enabled(en), owner(o), usage_ms(0) {}
	cpu(): cpuid(0), enabled(true), owner(NULL), usage_ms(0) {}

	void enable();
	void disable();
	void update_usage();

	inline bool operator<(const cpu& rhs) const {
		return cpuid < rhs.cpuid;
	}
	inline bool operator==(const cpu& rhs) const {
		return cpuid == rhs.cpuid;
	}
};


#endif
