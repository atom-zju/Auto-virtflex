#include "sample_queue.h"
#include "util.h"
#include <sys/time.h>
#include <unordered_map>
#include <iostream>

using namespace std;

//template<>
//unordered_map<string, unordered_map<string, unordered_map<string, vector<sample_queue<int>*>>>> sample_queue<int>::data_map;
//template<>
//unordered_map<string, unordered_map<string, unordered_map<string, vector<sample_queue<float>*>>>> sample_queue<float>::data_map;

template<>
int sample_queue<int>::get_sample(long long vm_start_time_sec_unix){
	if(!xs_dir.empty()){
		assert(vm_start_time_sec_unix);
		return get_sample_from_xs(vm_start_time_sec_unix);
	}
	else{
		if(get_sample_func_map.find(this->name) == get_sample_func_map.end()){
			cerr << "get_sample: cannot find get sample func in func map, name:" << this->name << endl;
			return -1;
		}
		else{
			return ((int (*)(sample_queue<int>*, node*))get_sample_func_map[this->name])(this, owner);
		}
	}
}

template<>
int sample_queue<float>::get_sample(long long vm_start_time_sec_unix){
        assert(xs_dir.empty());
        if(get_sample_func_map.find(this->name) == get_sample_func_map.end()){
        	cerr << "get_sample: cannot find get sample func in func map, name:" << this->name << endl;
                return -1;
     	}
        else{
                return ((int (*)(sample_queue<float>*, node*))get_sample_func_map[this->name])(this, owner);
        }
}


