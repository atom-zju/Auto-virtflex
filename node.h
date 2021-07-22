#ifndef NODE_H
#define NODE_H

#include <unordered_map>
#include <vector>
#include "sample_queue.h"
#include "cpu.h"


using namespace std; 

class node{
public:
	int pnode_id;
	int capacity;
	unordered_map<int, cpu> cpu_map;
	unordered_map<int, cpu>::iterator cpu_map_it;
	//vector<int> bw_rd;
	//vector<int> bw_wr;
	
        /* For the recording of bw history, use a deque of samples for each of the memory channel.
                Each sample has a time stamp (ms) (long long) and the bw measure (int). All the samples
                are sorted in chronological order. If the deque execeed the capacity, it will
                evict the oldest sample.
        */
        //vector<deque<pair<long long, int>>> bw_rd_channel_sample;
        //vector<deque<pair<long long, int>>> bw_wr_channel_sample;
	//int max_sample_size;
	vector<sample_queue<int>*> bw_rd_channel_sample;
	vector<sample_queue<int>*> bw_wr_channel_sample;
	cpu* first_cpu();	
	cpu* next_cpu();
	unsigned int ts;

	node(int id): pnode_id(id) {}
	node() {}
	~node();
};


#endif
