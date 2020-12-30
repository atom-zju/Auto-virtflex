#ifndef CPU_H
#define CPU_H

class vm;

class cpu{
public:
	int cpuid;
	bool enabled;
	vm* owner;

	cpu(int id, bool en, vm* o): cpuid(id), enabled(en), owner(o) {}

	void enable() const;
	void disable() const;

	inline bool operator<(const cpu& rhs) const {
		return cpuid < rhs.cpuid;
	}
	inline bool operator==(const cpu& rhs) const {
		return cpuid == rhs.cpuid;
	}
};


#endif
