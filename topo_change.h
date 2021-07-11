#ifndef TOPO_CHNAGE_H
#define TOPO_CHANGE_H

#include <vector>
#include <time.h>
#include <iostream>
#include <fstream>

/* sample queue related parameters */
//#define MAX_SAMPLE_SIZE 10
#define MAX_SAMPLE_SIZE 2000
#define VALID_SAMPLE_INTERVAL_MS 10000
#define SHORT_TERM_AVG_TS_SEC 60
#define LONG_TERM_AVG_TS_SEC 600

//#define UNIX_TS time(NULL)%1000000<<"\t" 
#define UNIX_TS time(NULL)<<" [ TOPO CHANGE DAEMON ] " 

extern ofstream of;
extern bool file_output;

typedef enum topo_changeness_cal_mthd {
	BW_load = 0,
	CPU_load = 1,
	BW_load_change = 2
} topo_changeness_cal_mthd;

typedef enum vnode_sq_type{
        VNODE_BW_RD_SQ = 0,
	VNODE_BW_WR_SQ = 1,
	VNODE_CPU_LOAD_SQ = 2
} vnode_sq_type;

//std::vector<string>  vnode_sq_paths{
//	"/bw_usage_rd",
//	"/bw_usage_wr",
//	"null"
//};

typedef enum vm_sq_type{
	VM_NUM_THREAD_SQ = 3
} vm_sq_type;

//std::vector<string>  vm_sq_paths{
//        "/numa/num_thread"
//};

typedef enum shrinkq_gen_mthd{
	SHRINK_FIRST_AVAILABLE = 0,
	BW_LEAST_USED = 1,
	CPU_LEAST_USED = 2	
} shrinkq_gen_mthd;

typedef enum expandq_gen_mthd{
	EXPAND_FIRST_AVAILABLE = 0
} expandq_gen_mthd;

#endif
