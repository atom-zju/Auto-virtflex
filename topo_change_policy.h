#ifndef TOPO_CHANGE_POLICY_H
#define TOPO_CHANGE_POLICY_H


enum topo_change_policy{
	max_performance_gain,
	max_topo_change_net_gain
};

// some possible policies: 
// preempt/no preempt, allow/not allow phsiscal node sharing, if allowed, how conservative should preempt be
// risky vs conservative? the margin of benefit vs cost is how large should the system do topo_change
// how should the system treat user resource requests? ignore/grant/credit based
// note: how vm id might change, so take care when load weight from config file
//
// exmaple of policy config file: ( format: <attr> = <val> )
// 	preempt = true/false
// 	preempt_margin = high/low/mid
// 	ignore_user_request = true/false
// 	w_vm1: critical/high/mid/low
// 	w_vm2: critical/high/mid/low

//class topo_change_policy{
//	private:
//		bool preempt;
//		bool ignore_user_request;
//		int preempt_margin;
//		int risk_margin;
//		string config_file;
//		unordered_map<int, int> vm_weight_map;
//	public:
//		topo_change_policy(){};
//		topo_change_policy(string config_file): config_file(config_file){};
//		void load_config();
//};
#endif
