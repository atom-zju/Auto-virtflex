#include "sample_queue.h"
#include "util.h"
#include <sys/time>

int sample_queue::get_smaple_from_xs(){
	string curr_sample_num;
	long long valid_ts = (time(0) - start_time)*1000 - VALID_SAMPLE_INTERVAL_MS;
	if(read_from_xenstore_path(xs, xs_dir, curr_sample_num) == 0){
		crawl_bw_samples_from_xs(xs, xs_dir, sample, max_sample_size);
	}
	else{
		cerr << "Sample dir may not exit: " << xs_dir << endl;
		return -1;
	}
}

int sample_queue::average_value_from_ts(long long valid_ts){
	long long sum = 0;
	int cnt = 0;
	for(auto& x: sample){
		sum+= x.second;
		cnt++;
	}
	return cnt? sum/(long long)cnt: -1;
}
