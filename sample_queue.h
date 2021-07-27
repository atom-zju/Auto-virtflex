#ifndef SAMPLE_Q_H
#define SAMPLE_Q_H

#include <deque>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>

//#define MAX_SAMPLE_SIZE 10
#define MAX_SAMPLE_SIZE 2000
#define VALID_SAMPLE_INTERVAL_MS 10000
#define SHORT_TERM_AVG_TS_SEC 60 
#define LONG_TERM_AVG_TS_SEC 600

#define TMP_DIR -2
#define TMP_NAME "temporary"
#define SYS_VM_ID -1
#define SYS_NODE_ID -1

#define CPU_USAGE_SQ "cpu sample queue"
#define NUM_OF_THREAD_SQ "num of threads sample queue"
#define BW_USAGE_SQ "bw sample queue"

using namespace std;

class node;

class dir{
	public:
	int vm;
	int node;
	dir(int vm=TMP_DIR, int node=TMP_DIR):vm(vm), node(node){}
};

bool is_guest_vm(int vm);
bool is_system_vm(int vm);
bool is_guest_node(int node);
bool is_system_node(int node);

template <class T>
class sample_queue{
public:
	deque<pair<long long, T>> sample;
	int max_sample_size;
	T long_term_average;
	T short_term_average;
	string xs_dir;
	string name;
	struct xs_handle *xs;
	node* owner;
	dir map_dir;	
	
	static unordered_map<int, unordered_map<int, unordered_map<string, vector<sample_queue<T> *>>>> data_map;
	//static vector<sample_queue<T>*> get_sq_from_data_map(string vm, string node, string metric); ///////
	static T calculate_avg(vector<sample_queue<T>*>& sqs, long long ux_ts_ms); ///////
	static vector<int> vm_list();
	static vector<int> vnode_list(int vm);
	
	sample_queue(string xs_dir, struct xs_handle *xs, dir md, string name = TMP_NAME,
				int max_size = MAX_SAMPLE_SIZE, node* owner=NULL); 
		//xs_dir(xs_dir), xs(xs), name(name), max_sample_size(max_size), owner(owner);
	~sample_queue();
	void calculate_averages();
	T get_short_average();
	T get_long_average();
	int get_sample(long long vm_start_time_sec_unix = 0);
	void clear_sample();
	T average_value_since_ts(long long valid_ts);
	void merge_sample_queue(sample_queue* s);
	pair<long long, T> get_nearst_data_point(long long ux_ts_ms);
	void insert_sample(pair<long long, T> p );
	void print(int num = 15);
private:	
	//static unordered_map<string, void(*)()> get_sample_func_map;
	int get_sample_from_xs(long long vm_start_time_sec_unix);
};

/* implementation of the template class */
#include "util.h"
#include <sys/time.h>
#include <iostream>

//template <class T>
//unordered_map<string, void (*)()> sample_queue<T>::get_sample_func_map = {
//	{"cpu sample queue", (void (*)())get_cpu_usage_sample}
//};

template<class T>
unordered_map<int, unordered_map<int, unordered_map<string, vector<sample_queue<T>*>>>> sample_queue<T>::data_map;

template<class T>
sample_queue<T>::sample_queue(string xs_dir, struct xs_handle *xs, dir md, string name,
		int max_size, node* owner): 
		xs_dir(xs_dir), xs(xs), name(name), max_sample_size(max_size), owner(owner), map_dir(md){
	data_map[map_dir.vm][map_dir.node][name].push_back(this);
}

template<class T>
sample_queue<T>::~sample_queue(){
	vector<sample_queue<T>*>& v = data_map[map_dir.vm][map_dir.node][name];
	for( int i=0; i < v.size(); i++)
		if(v[i] == this){
			v.erase(v.begin()+i);
			break;
		}
}

template<class T>
vector<int> sample_queue<T>::vm_list(){
	vector<int> res;
	for(auto& x: sample_queue<T>::data_map)
		if(is_guest_vm(x.first))
			res.push_back(x.first);
	return res;
}

template<class T>
vector<int> sample_queue<T>::vnode_list(int vm_id){
	vector<int> res;
	if(sample_queue<T>::data_map.find(vm_id) == sample_queue<T>::data_map.end())
		return res;
	for(auto& x: sample_queue<T>::data_map[vm_id])
		if(is_guest_node(x.first))
			res.push_back(x.first);
	return res;
}

template<class T>
T sample_queue<T>::calculate_avg(vector<sample_queue<T>*>& sqs, long long ux_ts_ms){
	T res = 0;
	int cnt = 0;
	for(auto& sq: sqs){
		T tmp = sq->average_value_since_ts(ux_ts_ms);
		if(tmp >= 0){
			res += tmp;
			cnt++;
		}
	}
	if(cnt == 0 )
		return -1;
	return res/(T)cnt;
}

template<class T>
void sample_queue<T>::print(int num){
	cout << " SQ Dir: vm:" << map_dir.vm << ", node: " << map_dir.node;
	cout << " SQ name: " << name;
	cout << " Content: ";
	int i = 0;
	if( sample.size() - num >= 0)
		i = sample.size() - num;
	for(; i < sample.size(); i++)
		cout << sample[i].first << ", " <<sample[i].second << " | ";
	cout << endl;
}

template <class T>
void sample_queue<T>::clear_sample(){
	this->sample.clear();
	long_term_average = -1;
	short_term_average = -1;	
}

template <class T >
void  sample_queue<T>::insert_sample(pair<long long, T> p ){
	int lo=0, hi = sample.size()-1;
	if(sample.empty() || p.first > sample[sample.size()-1].first){
		sample.push_back(p);
		return;
	}
	if(p.first < sample[0].first){
		sample.insert(sample.begin(), p);
		return;
	}
	while(lo <= hi){
		int mid = (hi-lo)/2+ lo;
		if(sample[mid].first == p.first)
			// duplicate sample, no need to insert
			return;
		if(p.first < sample[mid].first)
			hi = mid-1;
		else 
			lo = mid+1;
	}
	sample.insert(sample.begin()+lo, p);	
}
template<class T>
pair<long long, T> sample_queue<T>::get_nearst_data_point(long long ux_ts_ms){
	int lo = 0, hi= sample.size()-1;
	if(sample.empty())
		return make_pair(-1, -1);
	if(ux_ts_ms < sample[0].first)
		return sample[0];
	if(ux_ts_ms > sample[sample.size()-1].first)
		return sample[sample.size()-1];
	while(lo <= hi){
		int mid = (hi-lo)/2+lo;
		if(sample[mid].first == ux_ts_ms)
			return sample[mid];
		if(ux_ts_ms < sample[mid].first)
			hi = mid-1;
		else
			lo = mid+1;
	}
	if (ux_ts_ms - sample[hi].first <= sample[lo].first - ux_ts_ms)
		return sample[hi];
	return sample[lo];
}

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

//template<>
//int sample_queue<int>::get_sample(long long vm_start_time_sec_unix){
//	if(!xs_dir.empty()){
//		assert(vm_start_time_sec_unix);
//		return get_sample_from_xs(vm_start_time_sec_unix);
//	}
//	else{
//		if(get_sample_func_map.find(this->name) == get_sample_func_map.end()){
//			cerr << "get_sample: cannot find get sample func in func map, name:" << this->name << endl;
//			return -1;
//		}
//		else{
//			return ((int (*)(sample_queue<int>*, node*))get_sample_func_map[this->name])(this, owner);
//		}
//	}
//}
//
//template<>
//int sample_queue<float>::get_sample(long long vm_start_time_sec_unix){
//        assert(xs_dir.empty());
//        if(get_sample_func_map.find(this->name) == get_sample_func_map.end()){
//        	cerr << "get_sample: cannot find get sample func in func map, name:" << this->name << endl;
//                return -1;
//     	}
//        else{
//                return ((int (*)(sample_queue<float>*, node*))get_sample_func_map[this->name])(this, owner);
//        }
//}

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
	//cout << "cut off ts: " << valid_ts << endl;
	//cout << "valid sample cnt: " << cnt << endl;
	return cnt? sum/(T)cnt: -1;
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

#endif
