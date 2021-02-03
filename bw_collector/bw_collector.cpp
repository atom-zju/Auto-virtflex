#include "bw_collector.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>

using namespace std;

uint64_t rdmsr(int fd, uint64_t msr){
        uint64_t res;
	assert(fd!=-1);
        if(pread(fd, (void*)&res, sizeof(uint64_t), msr) < 0){
                std::cout << "Error when reading msr" << msr << std::endl;
                return 0;
        }
        return res;
}
int wrmsr(int fd, uint64_t msr, uint64_t value){
        uint64_t res;
	assert(fd!=-1);
        if(pwrite(fd, (void*)&value, sizeof(uint64_t), msr) < 0){
                std::cout << "Error when reading msr" << msr << std::endl;
                return -1;
        }
        return 0;
}

void bw_collector::init(){
	num_nodes = 4;
	pin_cpu_list = {0, 17, 33, 49};
	//num_nodes = 1;
	//pin_cpu_list = {0};
	
	config_msr_rd = {0xc0010242U, 0xc0010246U};
	config_msr_wr = {0xc0010240U, 0xc0010244U};
	config_val_rd = {0x403007U, 0x403047U};
	config_val_wr = {0x400807U, 0x400847U};

	value_msr_rd = {0xc0010243U, 0xc0010247U};
	value_msr_wr = {0xc0010241U, 0xc0010245U};

	for(auto x: pin_cpu_list){
		string path = "/dev/cpu/"+to_string(x)+"/msr";
		// path example: /dev/cpu/0/msr
		int fd = open(path.c_str(), O_RDWR | O_EXCL);
		if(fd == -1){
			perror("Error when open cpu msr file");
			exit(-1);
		}
		cpu_msr_fd.push_back(fd);
	}
}

void bw_collector::config_msr(){
	int err = 0;
	assert(config_msr_rd.size() == config_val_rd.size());
	assert(config_msr_wr.size() == config_val_wr.size());
	for(auto fd: cpu_msr_fd){
		for(int i=0; i<config_msr_rd.size(); i++){
			err |= wrmsr(fd, config_msr_rd[i], config_val_rd[i]);
		}
		for(int i=0; i<config_msr_wr.size(); i++){
			err |= wrmsr(fd, config_msr_wr[i], config_val_wr[i]);
		}
		for(int i=0; i<value_msr_rd.size(); i++){
			err |= wrmsr(fd, value_msr_rd[i], 0);
		}
		for(int i=0; i<value_msr_wr.size(); i++){
			err |= wrmsr(fd, value_msr_wr[i], 0);
		}
	}
	if(err)
		perror("wrmsr/rdmsr failed in config_msr");

	if(gettimeofday(&curr_ts, NULL))
		perror("gettimeofday failed");
}

void bw_collector::collect_val(){
	vector<vector<int>> bw_rd;
	vector<vector<int>> bw_wr;
	
	assert(num_nodes == cpu_msr_fd.size());
	bw_rd.resize(num_nodes);
	bw_wr.resize(num_nodes);
	
	prev_ts = curr_ts;	
	if(gettimeofday(&curr_ts, NULL))
		perror("gettimeofday failed");
	unsigned long time_epased_ms = (curr_ts.tv_sec - prev_ts.tv_sec)*1000 +
		(curr_ts.tv_usec - prev_ts.tv_usec)/1000;

	for(int i=0; i < cpu_msr_fd.size(); i++){
		for(int j=0; j<value_msr_rd.size(); j++){
			uint64_t bw = rdmsr(cpu_msr_fd[i], value_msr_rd[j]);
			wrmsr(cpu_msr_fd[i], value_msr_rd[j], 0);
			bw_rd[i].push_back(time_epased_ms? (bw*64/1024/1024)*1000/time_epased_ms: 0);
		}
		for(int j=0; j<value_msr_wr.size(); j++){
			uint64_t bw = rdmsr(cpu_msr_fd[i], value_msr_wr[j]);
			wrmsr(cpu_msr_fd[i], value_msr_wr[j], 0);
			bw_wr[i].push_back(time_epased_ms? (bw*64/1024/1024)*1000/time_epased_ms : 0);
		}
	}
	// display:
	for(int i=0; i < bw_rd.size(); i++){
		cout << "BW for node "<< i << endl;
		for(int j=0; j<bw_rd[i].size(); j++){
			cout << "\trd bw: " << bw_rd[i][j];
		}
		cout << endl;
		for(int j=0; j<bw_wr[i].size(); j++){
			cout << "\twr bw: " << bw_wr[i][j];
		}
		cout << endl;
	}

}

void bw_collector::clear_msr(){
	int err = 0;
	for(auto fd: cpu_msr_fd){
		for(int i=0; i<config_msr_rd.size(); i++){
			err |= wrmsr(fd, config_msr_rd[i], 0);
		}
		for(int i=0; i<config_msr_wr.size(); i++){
			err |= wrmsr(fd, config_msr_wr[i], 0);
		}
		for(int i=0; i<value_msr_rd.size(); i++){
			err |= wrmsr(fd, value_msr_rd[i], 0);
		}
		for(int i=0; i<value_msr_wr.size(); i++){
			err |= wrmsr(fd, value_msr_wr[i], 0);
		}
	}
	if(err)
		perror("wrmsr failed in clear_msr");
}

bw_collector::~bw_collector(){
	clear_msr();
	for(auto fd: cpu_msr_fd){
		close(fd);
	}
}
