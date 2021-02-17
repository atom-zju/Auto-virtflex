#ifndef SAMPLE_Q_H
#define SAMPLE_Q_H

#include <deque>
#include <string>

#define MAX_SAMPLE_SIZE 10
#define VALID_SAMPLE_INTERVAL_MS 10000

using namespace std;

class sample_queue{
public:
	deque<pair<long long, int>> sample;
	int max_sample_size;
	string xs_dir;
	string name;
	struct xs_handle *xs;

	sample_queue(string xs_dir, struct xs_handle *xs, string name, int max_size = MAX_SAMPLE_SIZE): 
		xs_dir(xs_dir), xs(xs), name(name), max_sample_size(max_size){};
	
	int get_smaple_from_xs(long long start_time_ms);
	int average_value_from_ts(long long valid_ts);
	void merge_sample_queue(sample_queue* s);
};

#endif
