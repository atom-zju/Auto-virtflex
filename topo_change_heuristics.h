#ifndef TOPO_CHANGE_HEURISTIC_H
#define TOPO_CHNAGE_HEURISTIC_H

enum heuristic_type{
	set_changeness_score,
	set_num_nodes
};

enum heristic_feedback{
	heu_fb_good,
	heu_fb_bad,
	heu_fb_netraul
};

class heristic_entry{
	
	// vm_snapshot, 
		// include currrent_num of active nodes, workload_feature
		// has to implement a similarity interface
	
	heuristic_type htype;
	
	// setting topo_changeness score or setting number of nodes to certain number
	union action{
		int topo_changness_delata;
		int number_of_nodes;
	} 
	
	float effective_score; 	// how effective this heristic entry is.
	
	int TTL; // changing the 
};

class topo_change_heuristic{
private:
	unordered_map<int, vector<heuristic_entry>> helper_h_map*;

	unordered_map<int, vector<heuristic_entry>> score_h_map;
	unordered_map<int, vector<heuristic_entry>> expand_h_map;
	unordered_map<int, vector<heuristic_entry>> shrink_h_map;

	// keep track of which heuristics are used, update heuristic scores	
	unordered_map<int, vector<heristic_entry*>> active_h_map; 
	
public:
	void insert_score_heu(int vm_id, vm_snapshot, action);
	void insert_expand_heu(int vm_id, vm_snapshot, action);
	void insert_shrink_heu(int vm_id, vm_snapshot, action);
	
	heuristic_entry query_score_heu(int vm_id, vm_snapshot);
	heuristic_entry query_epand_heu(int vm_id, vm_snapshot);
	heuristic_entry query_shrink_heu(int vm_id, vm_snapshot);

	void feedback_on_active_heu(int vm_id, heuritic_feedback heu_fb);
};

#endif
