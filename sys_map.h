#ifndef SYS_MAP_H
#define SYS_MAP_H 

#include <unordered_map>
#include <time.h>
#include <string>
#include <vector>
#include <cassert>

class topo_change_d;

using namespace std;

#define TOPO_SYS_MAP "topology_sys_map"			// base type: int, data_source: vnode_map
#define VCPU_USAGE_SYS_MAP "vcpu usage sys map"		// base type: float, data source: sample_queue::data_map
#define BW_USAGE_SYS_MAP "bw usage sys map"		// base type: int, data source: sq::data_map
#define NUM_THREAD_SYS_MAP "num of threads sys map"	// base type: int, data source: sq::data_map
#define NODE_SIZE_SYS_MAP "node size sys map"		// base type: int, data source: vnode_map
#define FREE_CLEAN_MEM_PERCNT "free and clean memory portion"	// base type: float, data source: sq::data_map

extern unordered_map<string, int (*)(topo_change_d*, void*, long long)> sys_map_func_map;

template<class T>
class map_record{
public:
	int vm_id;
	int vnode_id;
	int pnode_id;
	T data;
	bool empty;
	map_record(int vm_id, int vnode_id, int pnode_id, T data): 
			vm_id(vm_id), vnode_id(vnode_id), pnode_id(pnode_id), data(data), empty(false){}
	map_record():empty(true){}
	bool is_empty(){ return empty; }
};

class sys_map_base{
public:
	virtual string get_name() = 0;
	virtual vector<int> vm_list() = 0;
	virtual vector<int> vnode_list(int) = 0;
	virtual vector<int> pnode_list() = 0 ;
	virtual vector<int> vm_list_within_pnode(int) = 0 ;
	virtual int update(topo_change_d*, long long since_ux_ts_ms) = 0;
	//virtual map_record<T> vm_view(int vm_id, int vnode_id);
	//virtual map_record<T> pnode_view(int pnode_id, int vm_id);
};

template <class T>
class sys_map: public sys_map_base {
	public:
	string name;
	long long ux_ts_ms;
	vector<map_record<T>> records;/////////////////
	unordered_map<int, unordered_map<int, int>> vm_map;/////////////////
	unordered_map<int, unordered_map<int, int>> pnode_map;///////////////
	vector<int> vm_list(); 
	vector<int> vnode_list(int vm_id);
	vector<int> pnode_list();
	vector<int> vm_list_within_pnode(int pnode_id);
	sys_map(string name): name(name), ux_ts_ms(0){}
	sys_map(): ux_ts_ms(0){}
	void push_back(map_record<T> m);
	void del_entry_vm_view(int vm_id, int vnode_id);
	void print();
	string get_name();
	map_record<T>& vm_view(int vm_id, int vnode_id);
	map_record<T>& pnode_view(int pnode_id, int vm_id);
	void prune(sys_map<int>& topo_mask);
	void same_dimension_zero_fill(sys_map<int>& topo_mask);
	bool record_exist_vm_view(int vm_id, int vnode_id);
	bool record_exist_pnode_view(int pnode_id, int vm_id);
	void normalize(); /////////
	void mul(T scalar); ////////
	void mul(sys_map_base*); /////////
	void add(T scalar); ////////
	void add(sys_map_base* ); /////////
	void negate();
	T vm_avg(int vm_id, sys_map<int>* mask = NULL);///////
	T vm_sum(int vm_id, sys_map<int>* mask = NULL);///////
	T pnode_sum(int pnode_id, sys_map<int>* mask = NULL);////////
	int min_vnode_in_vm(int vm_id, sys_map<int>* mask = NULL);///////
	int min_vm_in_pnode(int pnode_id, sys_map<int>* mask = NULL);///////
	int max_vnode_in_vm(int vm_id, sys_map<int>* mask = NULL);////////
	int max_vm_in_pnode(int pnode_id, sys_map<int>* mask = NULL);//////////
	int update(topo_change_d* topod, long long since_ux_ts_ms=0);
};

template<class T>
void sys_map<T>::print(){
	cout<< "SYS_MAP: " << name << endl;
	for(auto& vm: vm_map){
		cout << "[=]vm: " << vm.first;
		for(auto& node: vm.second){
			cout << "   node " << node.first << ": ";
			if(records[node.second].is_empty())
				cout << "empty";
			else
				cout<<records[node.second].data;
		}
		cout << endl;
	}
}

template<class T>
string sys_map<T>::get_name(){
	return name;
}

template<class T>
void sys_map<T>::push_back(map_record<T> m){
	if(m.is_empty()){
		cout << "don't try to push back empty record" << endl;
		return;
	}
        records.push_back(m);
        //auto p = &(records[smap->records.size()-1]);
        vm_map[m.vm_id][m.vnode_id] = records.size()-1;
	//cout << "data:: " <<vm_view(m.vm_id,m.vnode_id).data <<endl;
       //cout << "data2: " << smap->vm_map[vm_id][vnode_id]->data << endl;
        pnode_map[m.pnode_id][m.vm_id] = records.size()-1;
}

template<class T>
bool sys_map<T>::record_exist_vm_view(int vm_id, int vnode_id){
	if(vm_map.find(vm_id) == vm_map.end())
		return false;
	if(vm_map[vm_id].find(vnode_id) == vm_map[vm_id].end())
		return false;
	return true;
}
template<class T>
bool sys_map<T>::record_exist_pnode_view(int pnode_id, int vm_id){
        if(pnode_map.find(pnode_id) == pnode_map.end())
                return false;
        if(pnode_map[pnode_id].find(vm_id) == pnode_map[pnode_id].end())
                return false;
        return true;
}

template<class T>
void sys_map<T>::negate(){
	for(auto& vm_id: vm_list()){
		for(auto& vnode_id: vnode_list(vm_id)){
			vm_view(vm_id, vnode_id).data = (vm_view(vm_id, vnode_id).data == 0);
		}
	}
}
template<class T>
void sys_map<T>::same_dimension_zero_fill(sys_map<int>& topo_mask){
	for(auto& vm_id: topo_mask.vm_list()){
		for(auto& vnode_id: topo_mask.vnode_list(vm_id)){
			if(!record_exist_vm_view(vm_id, vnode_id)){
				int pnode_id = topo_mask.vm_view(vm_id, vnode_id).pnode_id;
				map_record<T> m(vm_id, vnode_id, pnode_id, 0);
				push_back(m);
			}
		}	
	}
}

template<class T>
void sys_map<T>::del_entry_vm_view(int vm_id, int vnode_id){
	if(vm_map.find(vm_id) == vm_map.end() ||
		vm_map[vm_id].find(vnode_id) == vm_map[vm_id].end())
		return;
	int idx = vm_map[vm_id][vnode_id];
	auto r = records[idx];
	records.erase(records.begin()+idx);
	vm_map[vm_id].erase(vnode_id);
	if(vm_map[vm_id].size() == 0)
		vm_map.erase(vm_id);
	pnode_map[r.pnode_id].erase(r.vm_id);
	if(pnode_map[r.pnode_id].size() == 0)
		pnode_map.erase(r.pnode_id);
}

template<class T>
void sys_map<T>::mul(T scalar){
	for(auto& vm_id: vm_list())
		for(auto& vnode_id: vnode_list(vm_id)){
			vm_view(vm_id, vnode_id).data*= scalar;
		}
}


template<class T>
void sys_map<T>::add(T scalar){
	for(auto& vm_id: vm_list())
		for(auto& vnode_id: vnode_list(vm_id)){
			vm_view(vm_id, vnode_id).data += scalar;
		}
}

template<class T>
T sys_map<T>::vm_avg(int vm_id, sys_map<int>* mask){
	T res = 0;
	int cnt = 0;
	assert(vm_map.find(vm_id) != vm_map.end());
	for(auto& vnode_id: vnode_list(vm_id)){
		if(mask && mask->vm_view(vm_id, vnode_id).data != 0) {
			res += vm_view(vm_id, vnode_id).data;
			cnt++;	
		}
	}
	return cnt? res/(T)cnt: -1;
}
template<class T>
T sys_map<T>::vm_sum(int vm_id, sys_map<int>* mask){
	T res = 0;
	assert(vm_map.find(vm_id) != vm_map.end());
	for(auto& vnode_id: vnode_list(vm_id)){
		if(mask && mask->vm_view(vm_id, vnode_id).data != 0) {
			res += vm_view(vm_id, vnode_id).data;
		}
	}
	return res;
}
template<class T>
T sys_map<T>::pnode_sum(int pnode_id, sys_map<int>* mask){
        T res = 0;
        assert(pnode_map.find(pnode_id) != pnode_map.end());
        for(auto& vm_id: vm_list_within_pnode(pnode_id)){
		if(mask && mask->pnode_view(pnode_id, vm_id).data != 0) {
                	res += pnode_view(pnode_id, vm_id).data;
		}
        }
        return res;
}

template<class T>
int sys_map<T>::min_vnode_in_vm(int vm_id, sys_map<int>* mask){
	T min_res;
	int min_idx = -1;
	bool first = true;
	assert(vm_map.find(vm_id) != vm_map.end());
	for(auto& vnode_id: vnode_list(vm_id)){
		if(mask && mask->vm_view(vm_id, vnode_id).data == 0)
			continue;
		if(first){
			first = !first;
			min_res = vm_view(vm_id, vnode_id).data;
			min_idx = vnode_id;	
		}
		else if(vm_view(vm_id, vnode_id).data < min_res){
			min_res = vm_view(vm_id, vnode_id).data;
			min_idx = vnode_id;
		}
	}
	return min_idx;
}

template<class T>
int sys_map<T>::min_vm_in_pnode(int pnode_id, sys_map<int>* mask){
	return -1;
}
template<class T>
int sys_map<T>::max_vnode_in_vm(int vm_id, sys_map<int>* mask){
	T max_res;
	int max_idx = -1;
	bool first = true;
	assert(vm_map.find(vm_id) != vm_map.end());
	for(auto& vnode_id: vnode_list(vm_id)){
		if(mask && mask->vm_view(vm_id, vnode_id).data == 0)
			continue;
		if(first){
			first = !first;
			max_res = vm_view(vm_id, vnode_id).data;
			max_idx = vnode_id;	
		}
		else if(vm_view(vm_id, vnode_id).data > max_res){
			max_res = vm_view(vm_id, vnode_id).data;
			max_idx = vnode_id;
		}
	}
	return max_idx;
}
template<class T>
int sys_map<T>::max_vm_in_pnode(int pnode_id, sys_map<int>* mask){
	return -1;
}

template<class T>
vector<int> sys_map<T>::vm_list(){
	vector<int> res;
	for(auto& x: vm_map)
		res.push_back(x.first);
	return res;
}


template<class T>
void sys_map<T>::prune(sys_map<int>& topo_mask){
	//topo_mask.print();
	for(auto& vm_id: vm_list())
		for(auto& vnode_id: vnode_list(vm_id)){
			if(topo_mask.vm_view(vm_id, vnode_id).data == 0)
				del_entry_vm_view(vm_id, vnode_id); /////////
		}
}

template<class T>
vector<int> sys_map<T>::vnode_list(int vm_id){
	vector<int> res;
	if(vm_map.find(vm_id) == vm_map.end())
		return res;
	for(auto& x: vm_map[vm_id])
		res.push_back(x.first);
	return res;
}

template<class T>
vector<int> sys_map<T>::pnode_list(){
	vector<int> res;
	for(auto& x: pnode_map)
		res.push_back(x.first);
	return res;
}

template<class T>
vector<int> sys_map<T>::vm_list_within_pnode(int pnode_id){
	vector<int> res;
	if(pnode_map.find(pnode_id) == pnode_map.end())
		return res;
	for(auto& x: pnode_map[pnode_id])
		res.push_back(x.first);
	return res;
}

template<class T>
map_record<T>& sys_map<T>::vm_view(int vm_id, int vnode_id){
	assert(vm_map.find(vm_id) != vm_map.end() && 
		vm_map[vm_id].find(vnode_id) != vm_map[vm_id].end());
	return records[vm_map[vm_id][vnode_id]];
}

template<class T>
map_record<T>& sys_map<T>::pnode_view(int pnode_id, int vm_id){
	assert(pnode_map.find(pnode_id) != pnode_map.end() && 
			pnode_map[pnode_id].find(vm_id) != pnode_map[pnode_id].end());
	return records[pnode_view[pnode_id][vm_id]];
}

template<class T>
int sys_map<T>::update(topo_change_d* topod, long long since_ux_ts_ms){
	if (sys_map_func_map.find(name) == sys_map_func_map.end()){
		cerr << "sys_map<T>::update() err: cannot find func in sys_map_func_map" << endl;
		return -1;
	}
	//assert(typeid(T) == sys_map_func_map[name].second);
	vm_map.clear();
	pnode_map.clear();
	records.clear();
	ux_ts_ms = time(NULL)*1000;
	return sys_map_func_map[name](topod, this, since_ux_ts_ms);
}

#endif