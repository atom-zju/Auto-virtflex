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

int write_to_xenstore_path(struct xs_handle* xs, const string path, const string value);

int read_from_xenstore_path(struct xs_handle* xs, const string path, string& s);

int list_xenstore_directory(struct xs_handle* xs, const string path, vector<string>& res);

template <class T>
int return_insert_index(deque<pair<long long, T>>& samples, int lo, int hi, long long timestamp, bool* dup);

void crawl_bw_samples_from_xs(struct xs_handle * xs, string dir, deque<pair<long long, int>>& samples, int max_sample_size, long long start_time_ms);

//template <class T>
int get_cpu_usage_sample(sample_queue<float>* sq, node* n);

extern unordered_map<string, void(*)()> get_sample_func_map;

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
