#ifndef NODE_H
#define NODE_H

#include <unordered_map>
#include <vector>
#include <deque>
#include "cpu.h"

#define MAX_SAMPLE_SIZE 10
#define VALID_SAMPLE_INTERVAL_MS 2000

using namespace std; 

class node{
public:
	int pnode_id;
	int capacity;
	unordered_map<int, cpu> cpu_map;
	//vector<int> bw_rd;
	//vector<int> bw_wr;
	
        /* For the recording of bw history, use a deque of samples for each of the memory channel.
                Each sample has a time stamp (ms) (long long) and the bw measure (int). All the samples
                are sorted in chronological order. If the deque execeed the capacity, it will
                evict the oldest sample.
        */
        vector<deque<pair<long long, int>>> bw_rd_channel_sample;
        vector<deque<pair<long long, int>>> bw_wr_channel_sample;
	int max_sample_size;

	unsigned int ts;

	node(int id): pnode_id(id), max_sample_size(MAX_SAMPLE_SIZE) {}
	node(): max_sample_size(MAX_SAMPLE_SIZE) {}

};


#endif
