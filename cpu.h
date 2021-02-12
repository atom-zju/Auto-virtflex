#ifndef CPU_H
#define CPU_H
#include <cstddef>
#include <sys/time.h>
class vm;

class cpu{
public:
	int cpuid;
	bool enabled;
	vm* owner;
	struct timeval ts;
	long long usage_ms;
	long long recent_usage_ms;
	float recent_usage_percent;

	cpu(int id=0, bool en=true, vm* o=NULL): cpuid(id), enabled(en), owner(o), usage_ms(0) {}
	//cpu(): cpuid(0), enabled(true), owner(NULL), usage_ms(0) {}
	void init(struct timeval* now);

	void enable();
	void disable();
	void update_usage(struct timeval* now);
	//void update_usage();

	inline bool operator<(const cpu& rhs) const {
		return cpuid < rhs.cpuid;
	}
	inline bool operator==(const cpu& rhs) const {
		return cpuid == rhs.cpuid;
	}
};


#endif
