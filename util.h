#ifndef UTIL_H
#define UTIL_H

#include<vector>
#include<deque>
#include<string>
#include<unordered_map>
extern "C"
{
#include <xenstore.h>
#include <xenctrl.h>
#include <libxl.h>
}
template <class T>
class sample_queue;
class node;

using namespace std;

int write_to_xenstore_path(struct xs_handle* xs, const string path, 
		const string value);

int read_from_xenstore_path(struct xs_handle* xs, const string path, string& s);

int list_xenstore_directory(struct xs_handle* xs, const string path, 
		vector<string>& res);

bool xenstore_directory_is_valid(struct xs_handle* xs, const string path);

int get_workload_attr_from_xs(struct xs_handle* xs, const string xs_path, 
		unordered_map<string, long long>& attr_map);

template <class T>
int return_insert_index(deque<pair<long long, T>>& samples, int lo, int hi, 
		long long timestamp, bool* dup);

void crawl_samples_from_xs(struct xs_handle * xs, string dir, 
		deque<pair<long long, int>>& samples, int max_sample_size, 
		long long start_time_ms);

vector<pair<long long, string>> get_new_log_entries(struct xs_handle * xs, 
		string xs_dir, long long& last_ux_ts_ms, long long start_time_ms);

int get_cpu_usage_sample(sample_queue<float>* sq, node* n);

template<class T>
int floor_idx_of_sampleq(deque<pair<long long, T>>& samples, 
		int lo, int hi, long long timestamp){
	if( hi < lo || samples.empty() || hi >= samples.size()|| 
			timestamp < samples[lo].first)
		return -1;
	if(timestamp >= samples[hi].first)
		return hi;
	int ll = lo, hh = hi;
	while(ll <= hh){
		int mid = ll + (hh-ll)/2;
		if(samples[mid].first == timestamp)
			return mid;
		if(timestamp < samples[mid].first)
			hh = mid-1;
		else
			ll = mid+1;
	}
	return hh;
}

/*
	deque<pair<long long, int>>& samples are sorted in chronological order with the earliest sample in front
	This return_insert_index returns the idx the sample need to be inserted, and set the duplicate pointer to
	be true if already have samples of the same timestamps exist, duplicate pointer can be NULL
*/

template<class T >
int return_insert_index(deque<pair<long long, T>>& samples, int lo, int hi, long long timestamp, bool* duplicate){
	if(samples.empty()){
		if(duplicate)
			*duplicate = false;
		return 0;
	}
	if(lo == hi){
		if(samples[lo].first == timestamp){
			if(duplicate)
				*duplicate = true;
			return lo;
		}
		else if( timestamp < samples[lo].first){
			if(duplicate)
                                *duplicate = false;
			return lo;
		}
		else{
                        if(duplicate)
                                *duplicate = false;
			return lo+1;
		}
	}
	int mid = (hi - lo)/2 + lo;
	if (samples[mid].first == timestamp){
			if(duplicate)
				*duplicate = true;
			return mid;
	}
	else if (timestamp < samples[mid].first){
		if(mid == lo)
			return return_insert_index(samples, lo, mid, timestamp, duplicate);
		else
			return return_insert_index(samples, lo, mid-1, timestamp, duplicate);
	}
	else{
		if(mid == hi)
			return return_insert_index(samples, mid, hi, timestamp, duplicate);
		else
			return return_insert_index(samples, mid+1, hi, timestamp, duplicate);
	}
}

#endif
