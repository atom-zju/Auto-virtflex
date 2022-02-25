#include "util.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "topo_change.h"

using namespace std;


int write_to_xenstore_path(struct xs_handle* xs, const string path, const string value){
	
        xs_transaction_t th;

        int er;

        th = xs_transaction_start(xs);
        er = xs_write(xs, th, path.c_str(), value.c_str(), strlen(value.c_str()));
        xs_transaction_end(xs, th, false);

        if (er == 0){
                fprintf(stderr, "fail to write to path: %s\n", path.c_str());
                return -1;
        }
	
	return 0;

}

int read_from_xenstore_path(struct xs_handle* xs, const string path, string& s){
        
	xs_transaction_t th;

	char* buf;
	unsigned int len;
		
        th = xs_transaction_start(xs);
        buf = (char*)xs_read(xs, th, path.c_str(), &len);
        xs_transaction_end(xs, th, false);

        if (!buf){
                fprintf(stderr, "fail to read from path: %s\n", path.c_str());
                return -1;
        }
	s.clear();
	s.append(buf);	
	free(buf);
	return 0;

}

int list_xenstore_directory(struct xs_handle* xs, const string path, vector<string>& res){
	xs_transaction_t th;
	unsigned int num;
		
	char** list;
	th = xs_transaction_start(xs);
        list = xs_directory(xs, th, path.c_str(), &num);
        xs_transaction_end(xs, th, false);
	
	if(!list){
                fprintf(stderr, "fail to read xs directory: %s\n", path.c_str());
                return -1;
        }
	
	res.clear();	
	for(int i=0; i<num; i++){
		res.push_back(string(list[i]));
		//free(list[i]);
	}
	free(list);
	return 0;

}

bool xenstore_directory_is_valid(struct xs_handle* xs, const string path){
	vector<string> v;
	return list_xenstore_directory(xs, path, v) == 0;	
}


/*
 *	xenstore log dir: numa/log/curr_sample_num
 *				  /sample0
 *				  /sample0_ts
 *				  /sample1
 *				  /sample1_ts
 *				  ...
 *
 * */

/*
 * Effects: update last_ux_ts_mx to the newest timestamp
 * */

vector<pair<long long, string>> get_new_log_entries(struct xs_handle * xs, string dir, long long& last_ux_ts_ms, long long start_time_ms){
	vector<pair<long long, string>> res;
	string val_str;
	string timestamp_str;
	long long new_last_ts_ms = last_ux_ts_ms;
	int sample_cnt = 0;
	do{
		if(read_from_xenstore_path(xs, dir+"/sample"+to_string(sample_cnt), val_str) == 0){
			if(read_from_xenstore_path(xs, dir+"/sample"+to_string(sample_cnt)+"_ts", timestamp_str) == 0){
				long long ts;
				try{
					ts = stoll(timestamp_str);
				}
				catch(...){
					cerr << "stoll error in get_new_log_entries" << endl;
					sample_cnt++;
					continue;
				}
				cout << "ts: " << ts << endl;
				cout << "val_str: " << val_str << endl;
				//ts+= start_time_ms; // convert the xenstore uptime ts to global unix ts
				if(ts > last_ux_ts_ms){
					if(ts > new_last_ts_ms)
						new_last_ts_ms = ts;
					res.push_back({ts, val_str});
				}
			}
		}
		else{
			break;
		}
		sample_cnt++;
	} while(1);

	last_ux_ts_ms = new_last_ts_ms;	
	return res;
}

/*
        xenstore stats directory structure:
                numa/node/<node_id>/bw_usage_rd_0/curr_sample_num
                                                 /sample0
                                                 /sample0_ts
                                                 /sample1
                                                 /sample1_ts
                                                 ......
*/

///*
//	deque<pair<long long, int>>& samples are sorted in chronological order with the earliest sample in front
//	This return_insert_index returns the idx the sample need to be inserted, and set the duplicate pointer to
//	be true if already have samples of the same timestamps exist, duplicate pointer can be NULL
//*/
//
//int return_insert_index(deque<pair<long long, int>>& samples, int lo, int hi, long long timestamp, bool* duplicate){
//	if(samples.empty()){
//		if(duplicate)
//			*duplicate = false;
//		return 0;
//	}
//	if(lo == hi){
//		if(samples[lo].first == timestamp){
//			if(duplicate)
//				*duplicate = true;
//			return lo;
//		}
//		else if( timestamp < samples[lo].first){
//			if(duplicate)
//                                *duplicate = false;
//			return lo;
//		}
//		else{
//                        if(duplicate)
//                                *duplicate = false;
//			return lo+1;
//		}
//	}
//	int mid = (hi - lo)/2 + lo;
//	if (samples[mid].first == timestamp){
//			if(duplicate)
//				*duplicate = true;
//			return mid;
//	}
//	else if (timestamp < samples[mid].first){
//		if(mid == lo)
//			return return_insert_index(samples, lo, mid, timestamp, duplicate);
//		else
//			return return_insert_index(samples, lo, mid-1, timestamp, duplicate);
//	}
//	else{
//		if(mid == hi)
//			return return_insert_index(samples, mid, hi, timestamp, duplicate);
//		else
//			return return_insert_index(samples, mid+1, hi, timestamp, duplicate);
//	}
//}



void crawl_samples_from_xs(struct xs_handle * xs, string dir, deque<pair<long long, int>>& samples, int max_sample_size, long long start_time_ms){
	string val_str;
	string timestamp_str;
	int sample_cnt = 0;
	do{
		if(read_from_xenstore_path(xs, dir+"/sample"+to_string(sample_cnt), val_str) == 0){
			if(read_from_xenstore_path(xs, dir+"/sample"+to_string(sample_cnt)+"_ts", timestamp_str) == 0){
				long long ts;
				int val;
				try{
					ts = stoll(timestamp_str);
					val = stoi(val_str);
				}
				catch(...){
					cerr << "stoll or stoi error in crawl_bw_samples_from_xs" << endl;
					sample_cnt++;
					continue;
				}
				//ts+= start_time_ms; // convert the xenstore uptime ts to global unix ts
				bool dup;
				int idx =  return_insert_index(samples, 0, samples.size()-1, ts, &dup);
				if(!dup)
					samples.insert(samples.begin()+ idx, make_pair(ts, val));
				if(samples.size() > max_sample_size)
					samples.pop_front();
			}
		}
		else{
			break;
		}
		sample_cnt++;
	} while(1);

}

/* xenstore structure of workload_attr:
 *      /local/domain/#vm_id/numa/workload/
 *                                      attr0_name
 *                                      attr0_val
 *                                      attr1_name
 *                                      attr1_val
 * */

static int get_single_workload_attr_from_xs(struct xs_handle* xs, string xs_path, 
		int attr_num, pair<string, string>& ret){
	string attr_name_path = xs_path, attr_val_path = xs_path;
	attr_name_path.append("/attr"+to_string(attr_num)+"_name");	
	attr_val_path.append("/attr"+to_string(attr_num)+"_val");
	if(read_from_xenstore_path(xs, attr_name_path, ret.first) < 0)
		return -1;
	if(read_from_xenstore_path(xs, attr_val_path, ret.second) < 0)
                return -1;
	return 0;
}

int get_workload_attr_from_xs(struct xs_handle* xs, const string xs_path, unordered_map<string, long long>& attr_map){
	int cnter = 0;
	if(!xenstore_directory_is_valid(xs, xs_path))	
		return -1;
	pair<string, string> attr_pair;
	while(get_single_workload_attr_from_xs(xs, xs_path, cnter, attr_pair) >= 0){
		cnter++;
		long long val;
		// converstion from attr_pair.second to long long val
		try{
                       	val = stoll(attr_pair.second);
                }
                catch(...){
                        cerr << "stoll error in get_workload_attr_from_xs, string: " << attr_pair.second << endl;
                        continue;
                }
		attr_map[attr_pair.first] = val;
	}
	return 0;
}
