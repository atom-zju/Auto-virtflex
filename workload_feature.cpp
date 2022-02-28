#include "workload_feature.h"

void workload_feature::print(){
	cout << "workload_feature: ";
	cout << "cpu_avg_load:"  << cpu_avg_load << ", ";
	cout << "bw_avg_load: " << bw_avg_load << endl;
}

workload_feature::workload_feature(int vm_id, long long start_ux_ts_ms, 
		long long end_ux_ts_ms):vm_id(vm_id), start_ux_ts_ms(start_ux_ts_ms), 
		end_ux_ts_ms(end_ux_ts_ms){

	auto& int_data_map = sample_queue<int>::data_map;
	auto& float_data_map = sample_queue<float>::data_map;

	//default value
	cpu_avg_load = -1;
	bw_avg_load = -1;

	if(int_data_map.find(vm_id) != int_data_map.end()){
		// generate bw feature
		float avg;
		avg = sample_queue<int>::get_agg_avg_by_vm_id_sq_type(vm_id, 
				start_ux_ts_ms, end_ux_ts_ms, BW_USAGE_SQ);
		if(avg != -1)
			bw_avg_load = avg;
	}
	if(float_data_map.find(vm_id) != float_data_map.end()){
		// generate cpu feature
		float avg;
		avg = sample_queue<float>::get_agg_avg_by_vm_id_sq_type(vm_id,
				start_ux_ts_ms, end_ux_ts_ms, CPU_USAGE_SQ);
		if( avg != -1)
			cpu_avg_load = avg;
		
	}
}

// similarity is 0 --> no simlarity
// similarity is 1 --> identical workload
float workload_feature::similarity(workload_feature& feat){
	float similarity = 0;
	if(cpu_avg_load != -1 && feat.cpu_avg_load != -1){
		float diff = (cpu_avg_load - feat.cpu_avg_load) / max_cpu_avg_load;
		similarity += (1 - diff*diff) * cpu_load_weight;
	}
	if(bw_avg_load != -1 && feat.bw_avg_load != -1){
		float diff = (cpu_avg_load - feat.bw_avg_load)/ max_bw_avg_load;
		similarity += (1 - diff*diff)* bw_load_weight;		
	}
	return similarity;
}

const float workload_feature::max_cpu_avg_load = 1;
const float workload_feature::cpu_load_weight = 0.5;
const float workload_feature::max_bw_avg_load = 10000;
const float workload_feature::bw_load_weight = 0.5;
