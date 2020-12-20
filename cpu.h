#ifndef CPU_H
#define CPU_H


class cpu{
	int cpuid;
	bool enabled;

	cpu(int id): cpuid(id) {}

	void enable();
	void disable();

	inline bool operator<(const cpu& rhs) const {
		return cpuid < rhs.cpuid;
	}
	inline bool operator==(const cpu& rhs) const {
		return cpuid == rhs.cpuid;
	}
};


#endif
