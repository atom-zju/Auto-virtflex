#ifndef CPU_H
#define CPU_H


class cpu{
public:
	int cpuid;
	bool enabled;

	cpu(int id, bool en): cpuid(id), enabled(en) {}

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
