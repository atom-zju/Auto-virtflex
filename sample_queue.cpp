#include "sample_queue.h"
#include "util.h"
#include <sys/time.h>
#include <iostream>
template <class T>
unordered_map<string, void (*)()> sample_queue<T>::get_sample_func_map = {
	{"cpu sample queue", (void (*)())get_cpu_usage_sample}
};

/* Crawling samples from xenstore doesn't have any valid timestamp restrictions, 
   crawl it all. */
template <class T>
int sample_queue<T>::get_sample_from_xs(long long vm_start_time_sec_unix){
	if(!xs)
		return -1;
	if(!is_same<T, int>::value){
          	cout << "get_sample_from_xs: type error" << endl;
          	return -1;
      	}
	string curr_sample_num;
	//long long valid_ts = (time(0) - vm_start_time_sec_unix)*1000 - VALID_SAMPLE_INTERVAL_MS;
	if(read_from_xenstore_path(xs, xs_dir, curr_sample_num) == 0){
		crawl_bw_samples_from_xs(xs, xs_dir, sample, max_sample_size, vm_start_time_sec_unix*1000);
		return 0;
	}
	else{
		cerr << "Sample dir may not exit: " << xs_dir << endl;
		return -1;
	}
}


template <class T>
int sample_queue<T>::get_sample(long long vm_start_time_sec_unix){
	if(!xs_dir.empty()){
		return get_sample_from_xs(vm_start_time_sec_unix);
	}
	else{
		if(get_sample_func_map.find(this->name) == get_sample_func_map.end()){
			cerr << "get_sample: cannot find get sample func in func map, name:" << this->name << endl;
			return -1;
		}
		else{
			return ((int (*)(sample_queue<T>*, node*))get_sample_func_map[this->name])(this, owner);
		}
	}	
}

template <class T>
T sample_queue<T>::average_value_since_ts(long long valid_ts){
	int idx;
	bool dup;
	idx = return_insert_index(sample, 0, sample.size()-1, valid_ts, &dup);
	T sum = 0;
	int cnt = 0;
	for(int i = idx; i < sample.size(); i++){
		sum+= sample[i].second;
		cnt++;
	}
	return cnt? sum/(long long)cnt: -1;
}

template <class T>
void sample_queue<T>::calculate_averages(){
	// TODO: calculate the short and long term averages
	short_term_average = average_value_since_ts((time(0)-SHORT_TERM_AVG_TS_SEC)*1000); 
	long_term_average = average_value_since_ts((time(0)-LONG_TERM_AVG_TS_SEC)*1000);
}

template <class T>
T sample_queue<T>::get_short_average(){
	return short_term_average;
}

template <class T>
T sample_queue<T>::get_long_average(){
	return long_term_average;
}

/* Note: 
	1. calling this function for large sample size might be costly
	2. calling this function repeatitively might result in redudant work*/
template <class T>
void sample_queue<T>::merge_sample_queue(sample_queue* s){
	for(auto& x: s->sample){
		bool dup;
		int idx = return_insert_index(sample, 0, sample.size()-1, x.first, &dup);
		if (!dup){
			sample.insert(sample.begin()+idx, x);
		}
		if(sample.size() > max_sample_size)
			sample.pop_front();
	}
}
