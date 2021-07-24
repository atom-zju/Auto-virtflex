#ifndef PNODE_H
#define PNODE_H
#include <unordered_map>
#include <vector>
#include <deque>
#include "node.h"
#include "vnode.h"

using namespace std;

#define HISTORY_LEN 10

class pnode: public node {
public:
	int total_vnodes;
	int active_vnodes;
	long long recent_cpu_usage;
	vnode* owner_vnode;
	unordered_map<int, vnode*> vnode_map;
	vector<sample_queue<int>*> bw_rd_sq_grabber;
	vector<sample_queue<int>*> bw_wr_sq_grabber;
	
	/* For the recording of bw history, use a deque of samples for each of the memory channel.
		Each sample has a time stamp (long long) and the bw measure (int). All the samples
		are sorted in chronological order. If the deque execeed the capacity, it will
		evict the oldest sample.
	*/
	//vector<deque<pair<long long, int>>> bw_rd_channel_sample; 
	//vector<deque<pair<long long, int>>> bw_wr_channel_sample;
	//int max_sample_count;
	 
	pnode(int id); 
	~pnode();		
	void register_vnode(int vm_id, vnode* n);
	void unregister_vnode(int vm_id, vnode* n);
	void update_vnode_map(int ts);
	void pick_owner_vnode(void);
	void get_owner_vnode_stat(void);
	long average_bw_usage();
	//void copy_owner_vnode_bw_usage();
	void grab_bw_sample();
	void scatter_bw_sample();
};

#endif
