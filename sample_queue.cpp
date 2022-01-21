#include "sample_queue.h"
#include "util.h"
#include "cpu.h"
#include "node.h"
#include "vm.h"
#include <sys/time.h>
#include <unordered_map>
#include <iostream>

using namespace std;


bool is_guest_vm(int vm){ return vm > 0; }
bool is_system_vm(int vm){ return vm == SYS_VM_ID;}
bool is_guest_node(int node){ return node >= 0;}
bool is_system_node(int node){ return node == SYS_NODE_ID; }

//template <class T>
int get_cpu_usage_sample(void* op1, void* op2, long long vm_start_time_sec_unix){
	assert(op1 && op2);
	auto sq = (sample_queue<float>*)op1;
	auto owner = (node*)op2;
	if(!owner){
		cerr<< "get_cpu_usage_sample using a NULL node pointer" << endl;
		return -1;
	}
	//if(!is_same<T,float>::value){
	//	cerr<< "get_cpu_usage_sample base type is not float" <<endl;
	//	return -1;
	//}
	vector<pair<long long,float>> merge;
	int cnt = 0;
	cpu* cpu_ptr = owner->first_cpu();
	do{
		const deque<pair<long long,float>>& q = cpu_ptr->samples;
		for(int i=0; i<q.size(); i++){
			if(i >= merge.size()){
				merge.push_back(make_pair(0, 0));
			}
			//assert(merge[i].first==0 || merge[i].first == q[i].first);
			if( merge[i].first == 0)
				merge[i].first = q[i].first;
			assert(merge[i].first == q[i].first);
			merge[i].second+=q[i].second;
		}
		cnt++;
	}while(cpu_ptr = owner->next_cpu());
	for(int i = 0; i < merge.size(); i++){
		merge[i].second /= cnt;
	}
        for(auto& m: merge){
                bool dup;
                int idx = return_insert_index(sq->sample, 0, sq->sample.size()-1, m.first, &dup);
                if (!dup){
                        sq->sample.insert(sq->sample.begin()+idx, m);
                }
                if(sq->sample.size() > sq->max_sample_size)
                        sq->sample.pop_front();
        }
	return 0;
}

int get_sample_from_xs(void* op1, void* op2, long long vm_start_time_sec_unix){
	assert(op1 && vm_start_time_sec_unix);
	auto sq = (sample_queue<int>*)op1;
	assert(sq->xs && !sq->xs_dir.empty());
	string curr_sample_num;
      	//long long valid_ts = (time(0) - vm_start_time_sec_unix)*1000 - VALID_SAMPLE_INTERVAL_MS;
      	if(read_from_xenstore_path(sq->xs, sq->xs_dir, curr_sample_num) == 0){
      	        crawl_samples_from_xs(sq->xs, sq->xs_dir, sq->sample, 
				sq->max_sample_size, vm_start_time_sec_unix*1000);
      	        return 0;
      	}
      	else{
      	        cerr << "Sample dir may not exit: " << sq->xs_dir << endl;
      	        return -1;
      	}
}


int get_idleness_sample(void* op1, void* op2, long long vm_start_time_sec_unix){
	assert(op1 && op2);
	auto sq = (sample_queue<int>*)op1;
	auto owner_vm = (vm*)op2;
	long long ts = time(0)*1000;
	int idleness = owner_vm->is_running_workload();

	if(sq->sample.empty() || sq->sample.back().second != idleness)
		sq->sample.push_back({ts, idleness});
	return 0;
}

int get_num_active_node_sample(void* op1, void* op2, long long vm_start_time_sec_unix){
	assert(op1 && op2);
	auto sq = (sample_queue<int>*)op1;
	auto owner_vm = (vm*)op2;
	long long ts = time(0)*1000;
	int num_active_node = owner_vm->active_node;

	if(sq->sample.empty() || sq->sample.back().second != num_active_node)
		sq->sample.push_back({ts, num_active_node});
	return 0;
}

unordered_map<string, int (*)(void*, void*, long long)> get_sample_func_map = {
        { CPU_USAGE_SQ, get_cpu_usage_sample },
	{ BW_USAGE_SQ, get_sample_from_xs},
	{ NUM_OF_THREAD_SQ, get_sample_from_xs},
	{ IDLENESS_SQ, get_idleness_sample },
	{ NUM_ACTIVE_NODE_SQ, get_num_active_node_sample}
};
