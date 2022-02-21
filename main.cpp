#include <csignal>
#include "topo_change_d.h"

bool file_output = false;
ofstream of;
topo_change_d* topod_ptr = NULL;

void print_usage(){
	cout<< "Usage: sudo ./topo_change [output_filename]" << endl;
}

void signal_handler(int signum){
	cout << "Interrupt signal recieved, exiting with signum " << signum << endl;
	assert(topod_ptr);
	topod_ptr->interrupt();	
}

int main(int argc, char** argv){
	if(argc >= 2){
		of.open(argv[1]);
		if(!of.is_open()){
			cout<< "cannnot open file" << argv[1] << endl;
			print_usage();
			exit(-1);
		}
		file_output = true;
	}
	topo_change_d topod;
	topod_ptr = &topod;
	signal(SIGINT, signal_handler);
       	topod.set_interval_ms(1000);
       	topod.start();
       	//topod.update_vm_map();
       	//topod.expand_vm(2, 3);
	if(file_output){
		of.close();
	}
	return 0;
}
