#include "node.h"

node::~node(){
	for(auto& x : bw_rd_channel_sample){
		delete x;
	}
	for(auto& x : bw_wr_channel_sample){
                delete x;
        }
}
