#include "node.h"

node::~node(){
	for(auto& x : bw_rd_channel_sample){
		delete x;
	}
	for(auto& x : bw_wr_channel_sample){
                delete x;
        }
}

cpu* node::first_cpu(){
	cpu_map_it = cpu_map.begin();
	return &(cpu_map_it->second);
}
cpu* node::next_cpu(){
	cpu_map_it++;
	if(cpu_map_it == cpu_map.end())
		return NULL;
	return &(cpu_map_it->second);
}
