#ifndef UTIL_H
#define UTIL_H

#include<vector>
#include<deque>
#include<string>
extern "C"
{
#include <xenstore.h>
#include <xenctrl.h>
#include <libxl.h>
}

using namespace std;

int write_to_xenstore_path(struct xs_handle* xs, const string path, const string value);

int read_from_xenstore_path(struct xs_handle* xs, const string path, string& s);

int list_xenstore_directory(struct xs_handle* xs, const string path, vector<string>& res);

int return_insert_index(deque<pair<long long, int>>& samples, int lo, int hi, long long timestamp, bool* dup);

void crawl_bw_samples_from_xs(struct xs_handle * xs, string dir, deque<pair<long long, int>>& samples, int max_sample_size, long long start_time_ms);

#endif
