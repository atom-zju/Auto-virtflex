#ifndef WORKLOAD_FEATURE_H
#define WORKLOAD_FEATURE_H

#include "sample_queue.h"


class workload_feature{
	private:
		int vm_id;
		long long start_ux_ts_ms;
		long long end_ux_ts_ms;
		float cpu_avg_load;
		float bw_avg_load;
	public:
		// static variables
		static const float max_cpu_avg_load;
		static const float cpu_load_weight;
		static const float max_bw_avg_load;
		static const float bw_load_weight;
		
		workload_feature(int vm_id, long long start_ts, long long end_ts);
		float similarity(workload_feature& feat);
		void print();
};

#endif
