#include "sample_queue.h"
#include "util.h"
#include <sys/time.h>
#include <iostream>

int sample_queue::get_smaple_from_xs(long long start_time_ms){
	if(!xs)
		return -1;
	string curr_sample_num;
	long long valid_ts = (time(0) - start_time_ms)*1000 - VALID_SAMPLE_INTERVAL_MS;
	if(read_from_xenstore_path(xs, xs_dir, curr_sample_num) == 0){
		crawl_bw_samples_from_xs(xs, xs_dir, sample, max_sample_size, start_time_ms);
		return 0;
	}
	else{
		cerr << "Sample dir may not exit: " << xs_dir << endl;
		return -1;
	}
}

int sample_queue::average_value_from_ts(long long valid_ts){
	int idx;
	bool dup;
	idx = return_insert_index(sample, 0, sample.size()-1, valid_ts, &dup);
	long long sum = 0;
	int cnt = 0;
	for(int i = idx; i < sample.size(); i++){
		sum+= sample[i].second;
		cnt++;
	}
	return cnt? sum/(long long)cnt: -1;
}

/* Note: 
	1. calling this function for large sample size might be costly
	2. calling this function repeatitively might result in redudant work*/
void sample_queue::merge_sample_queue(sample_queue* s){
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
