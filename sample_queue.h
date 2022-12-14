#ifndef SAMPLE_Q_H
#define SAMPLE_Q_H

#include <deque>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#include "topo_change.h"

//#define MAX_SAMPLE_SIZE 10
#define MAX_SAMPLE_SIZE 2000
#define VALID_SAMPLE_INTERVAL_MS 10000
#define SHORT_TERM_AVG_TS_SEC 60 
#define LONG_TERM_AVG_TS_SEC 600

//#define TMP_DIR -2
//#define TMP_NAME "temporary"
//#define SYS_VM_ID -1
//#define SYS_NODE_ID -1
//#define INVALID_SQ -3

#define CPU_USAGE_SQ "cpu sample queue"
#define NUM_OF_THREAD_SQ "num of threads sample queue"
#define BW_USAGE_SQ "bw sample queue"
#define IDLENESS_SQ "idleness sample queue"
#define NUM_ACTIVE_NODE_SQ "num of active node sameple queue"

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
	void* owner;
	dir map_dir;	
	
	static unordered_map<int, unordered_map<int, unordered_map<string, vector<sample_queue<T> *>>>> data_map;
	//static vector<sample_queue<T>*> get_sq_from_data_map(string vm, string node, string metric); ///////
	static T calculate_avg(vector<sample_queue<T>*>& sqs, long long ux_ts_ms); ///////
	static T last_record_avg(vector<sample_queue<T>*>& sqs);
	static vector<int> vm_list();
	static vector<int> vnode_list(int vm);
	static bool has_sys_vm();
	static bool has_sys_node(int vm);
	static float get_agg_avg_by_vm_id_sq_type(int vm_id, long long start_ts_ms, 
			long long end_ts_ms, string sq_type);
	
	sample_queue(string xs_dir, struct xs_handle *xs, dir md, string name = TMP_NAME,
				 void* owner=NULL, int max_size = MAX_SAMPLE_SIZE); 
		//xs_dir(xs_dir), xs(xs), name(name), max_sample_size(max_size), owner(owner);
	~sample_queue();
	void calculate_averages();
	T get_short_average();
	T get_long_average();
	int get_sample(long long vm_start_time_sec_unix = 0);
	void clear_sample();
	T average_value_since_ts(long long valid_ts);
	float range_average(long long start_ux_ts_ms, long long end_ux_ts_ms);
	pair<long long, T> last_record();
	void merge_sample_queue(sample_queue* s);
	pair<long long, T> get_nearst_data_point(long long ux_ts_ms);
	void insert_sample(pair<long long, T> p );
	void print(int num = 15);
private:	
	//static unordered_map<string, void(*)()> get_sample_func_map;
};

/* implementation of the template class */
#include "util.h"
#include <sys/time.h>
#include <iostream>

extern unordered_map<string, int(*)(void*, void*, long long)> get_sample_func_map;

template<class T>
unordered_map<int, unordered_map<int, unordered_map<string, vector<sample_queue<T>*>>>> sample_queue<T>::data_map;

template<class T>
sample_queue<T>::sample_queue(string xs_dir, struct xs_handle *xs, dir md, string name,
		void* owner, int max_size): 
		xs_dir(xs_dir), xs(xs), name(name), max_sample_size(max_size), owner(owner), map_dir(md){
	data_map[map_dir.vm][map_dir.node][name].push_back(this);
}

template<class T>
sample_queue<T>::~sample_queue(){
	if(data_map.find(map_dir.vm) == data_map.end() ||
			data_map[map_dir.vm].find(map_dir.node) == data_map[map_dir.vm].end() ||
			data_map[map_dir.vm][map_dir.node].find(name) == 
			data_map[map_dir.vm][map_dir.node].end())
		return;
	vector<sample_queue<T>*>& v = data_map[map_dir.vm][map_dir.node][name];
	for( int i=0; i < v.size(); i++)
		if(v[i] == this){
			v.erase(v.begin()+i);
			break;
		}
}

template<class T>
float sample_queue<T>::get_agg_avg_by_vm_id_sq_type(int vm_id, long long start_ts_ms, 
		long long end_ts_ms, string sq_type){
	float res = 0;
	int cnt = 0;
	if(data_map.find(vm_id) == data_map.end()){
		return -1;
	}
	for(auto& x: data_map[vm_id]){
		// for each node
		//if(x.first == SYS_NODE_ID)
		//	continue;
		auto& node_map = x.second;
		if(node_map.find(sq_type) == node_map.end())
			continue;
		for(auto& y: node_map[sq_type]){
			// for each sq pointer
			assert(y);
			auto avg = y->range_average(start_ts_ms, end_ts_ms);
			if((float)avg < 0)
				continue;
			res += (float)avg;
			cnt++;	
		}
		
	}
	if(cnt == 0)
		return -1;
	return res/cnt;
}

template<class T>
bool sample_queue<T>::has_sys_vm(){
	if(sample_queue<T>::data_map.find(SYS_VM_ID) == sample_queue<T>::data_map.end())
		return false;
	return true;
}
template<class T>
bool sample_queue<T>::has_sys_node(int vm){
	if(sample_queue<T>::data_map.find(vm) == sample_queue<T>::data_map.end() ||
			sample_queue<T>::data_map[vm].find(SYS_NODE_ID) == sample_queue<T>::data_map[vm].end())
		return false;
	return true;
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
T sample_queue<T>::last_record_avg(vector<sample_queue<T>*>& sqs){
	T res = 0;
	int cnt = 0;
	for(auto& sq: sqs){
                auto tmp = sq->last_record();
                if(tmp.first >= 0 && tmp.second > 0){
                        res += tmp.second;
                        cnt++;
                }
        }
        if(cnt == 0 )
                return -1;
        return res/(T)cnt;	
}
template<class T>
pair<long long, T> sample_queue<T>::last_record(){
	if(sample.empty())
		return make_pair(-1, -1);
	return sample[sample.size()-1];
}

template<class T>
void sample_queue<T>::print(int num){
	cout << " SQ Dir: vm:" << map_dir.vm << ", node: " << map_dir.node;
	cout << " SQ name: " << name;
	cout << " Content: ";
	int i = 0;
	if( (int)sample.size() - num > 0)
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

template<class T>
int sample_queue<T>::get_sample(long long vm_start_time_sec_unix){
	if(get_sample_func_map.find(this->name) == get_sample_func_map.end()){
		cerr << "get_sample: cannot find get sample func in func map, name:" << this->name << endl;
		return -1;
	}
	else{
		return (get_sample_func_map[this->name])((void*)this, 
				owner, vm_start_time_sec_unix);
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
	//cout << "cut off ts: " << valid_ts << endl;
	//cout << "valid sample cnt: " << cnt << endl;
	return cnt? sum/(T)cnt: -1;
}

template<class T>
float sample_queue<T>::range_average(long long start_ux_ts_ms, long long end_ux_ts_ms){
	int start_idx, end_idx;
	bool dup;
	float res = 0;
	int cnt = 0;
	
	assert(start_ux_ts_ms < end_ux_ts_ms);
	start_idx = return_insert_index(sample, 0, sample.size()-1, start_ux_ts_ms, &dup);
	end_idx = return_insert_index(sample, min(start_idx, (int)sample.size()-1), 
			sample.size()-1, end_ux_ts_ms, &dup);
	if(start_idx < 0 || end_idx < 0)
		return -1;
	for(int i=start_idx; i<end_idx && i<sample.size()-1; i++){
		res += (float)sample[i].second;
		cnt++;
	}
	if(cnt == 0)
		return -1;
	return res/(T)cnt;
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
