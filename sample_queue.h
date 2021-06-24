#ifndef SAMPLE_Q_H
#define SAMPLE_Q_H

#include <deque>
#include <string>

//#define MAX_SAMPLE_SIZE 10
#define MAX_SAMPLE_SIZE 2000
#define VALID_SAMPLE_INTERVAL_MS 10000
#define SHORT_TERM_AVG_TS_SEC 60 
#define LONG_TERM_AVG_TS_SEC 600

using namespace std;

class sample_queue{
public:
	deque<pair<long long, int>> sample;
	int max_sample_size;
	long long long_term_average;
	long long short_term_average;
	string xs_dir;
	string name;
	struct xs_handle *xs;

	sample_queue(string xs_dir, struct xs_handle *xs, string name, int max_size = MAX_SAMPLE_SIZE): 
		xs_dir(xs_dir), xs(xs), name(name), max_sample_size(max_size){};

	void calculate_averages();
	long long get_short_average();
	long long get_long_average();
	int get_sample_from_xs(long long vm_start_time_sec_unix);
	int average_value_since_ts(long long valid_ts);
	void merge_sample_queue(sample_queue* s);
};

#endif
