#ifndef TOPO_CHNAGE_H
#define TOPO_CHANGE_H

#include <vector>
#include <time.h>
#include <iostream>
#include <fstream>

/* sample queue related parameters */
//#define MAX_SAMPLE_SIZE 10
//#define MAX_SAMPLE_SIZE 2000
//#define VALID_SAMPLE_INTERVAL_MS 10000
//#define SHORT_TERM_AVG_TS_SEC 60
//#define LONG_TERM_AVG_TS_SEC 600
#define MAX_LOCAL_CPU_SP_SIZE 10

#define TMP_DIR -2
#define TMP_NAME "temporary"
#define SYS_VM_ID -1
#define SYS_NODE_ID -1
#define INVALID_SQ -3

//#define CPU_USAGE_SQ "cpu sample queue"
//#define NUM_OF_THREAD_SQ "num of threads sample queue"
//#define BW_USAGE_SQ "bw sample queue"

//#define UNIX_TS time(NULL)%1000000<<"\t" 
#define UNIX_TS time(NULL)<<" [ TOPO CHANGE DAEMON ] " 

// define the length of topochangeness caldulation time window in seconds
#define TOPO_CHNAGENESS_TIME_WINDOW_SECS 5

extern std::ofstream of;
extern bool file_output;

#endif
