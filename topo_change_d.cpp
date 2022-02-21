#include <iostream>
#include <unistd.h>
#include <cassert>
#include <linux/kernel.h>       /* for struct sysinfo */
#include <sys/sysinfo.h>

#include "topo_change_d.h"
#include "util.h"
#include "topo_change_engine.h"
#include "vm_logger.h"

using namespace std;

void topo_change_d::set_interval_ms(unsigned int ms){
	interval_us = ms*1000;
}

vm* topo_change_d::get_vm_by_id(int id){
	if(vm_map.find(id) != vm_map.end()){
		return vm_map[id];
	}
	return NULL;
}

vector<int> topo_change_d::vm_list(){
	vector<int> res;
	for(auto& x: vm_map){
		if(x.first != 0)
			res.push_back(x.first);
	}
	return res;
}

string topo_change_d::get_xs_path(){
	return xs_path;
}

int topo_change_d::vnode_to_pnode(int vm_id, int vnode_id){
	if(vm_map.find(vm_id) == vm_map.end() || 
		vm_map[vm_id]->vnode_map.find(vnode_id) == vm_map[vm_id]->vnode_map.end())
		return -2;
	return vm_map[vm_id]->vnode_map[vnode_id]->pnode_id;
}

int topo_change_d::pnode_to_vnode(int vm_id, int pnode_id){
        if(pnode_id >= pnode_list.size() ||
                pnode_list[pnode_id]->vnode_map.find(vm_id) == pnode_list[pnode_id]->vnode_map.end())
                return -2;
	return pnode_list[pnode_id]->vnode_map[vm_id]->vnode_id;
}

int topo_change_d::reserved_vnode_id(int vm_id){
	if(vm_map.find(vm_id) == vm_map.end())
		return -1;
	return vm_map[vm_id]->reserved_vnode_id;
}
int topo_change_d::set_reserved_vnode_id(int vm_id, int vnode_id){
	if(vm_map.find(vm_id) == vm_map.end())
		return -1;
	vm_map[vm_id]->reserved_vnode_id = vnode_id;
	return 0;
}
topo_change_d::topo_change_d():interrupted(false){
	ts = 0;
	xs =  xs_daemon_open();
	xs_path = "/local/domain";
        if ( xs == NULL ) {
                perror("Error when connecting to a xs daemon\n");
                exit(-1);
        }
	xc_handle = xc_interface_open(NULL, NULL, 0);
	if(!xc_handle){
                perror("xc_interface_open failed");
                exit(-1);
        }
	xl_logger = xtl_createlogger_stdiostream(stderr, XTL_PROGRESS,
                    XTL_STDIOSTREAM_PROGRESS_USE_CR);
	if(!xl_logger){
                perror("Error in libxl_ctx_alloc");
                exit(-1);
        }
	libxl_ctx_alloc(&xl_handle, LIBXL_VERSION, 0 , (xentoollog_logger*)xl_logger);
        if(!xl_handle){
                perror("Error in libxl_ctx_alloc");
                exit(-1);
        }
	engine = new topo_change_engine(this);
	assert(engine);
	engine->config();
}

topo_change_d::~topo_change_d(){
	// free pointers in vm_map and pnode_list
	for(auto& x: vm_map){
		delete x.second;
	}

	for(auto& x: pnode_list){
		delete x;
	}
	xs_daemon_close(xs);
	xc_interface_close(xc_handle);
        libxl_ctx_free(xl_handle);
        xtl_logger_destroy((xentoollog_logger*)xl_logger);
	delete engine;
}

void topo_change_d::update_vm_map(){
	ts++;
	vector<string> dir;
	list_xenstore_directory(xs, xs_path, dir);
	//cout<<"/local/domain"<< endl;
	cout << UNIX_TS<< "\tUpdating vm_map" << endl;
	if(file_output)
	of << UNIX_TS<< "\tUpdating vm_map" << endl;
	for(auto& x: dir){
		int vm_id =  stoi(x);
		cout << UNIX_TS<<"\t\t" <<"VM: "<<stoi(x) << endl;
		if(file_output)
		of << UNIX_TS<<"\t\t" <<"VM: "<<stoi(x) << endl;
		if(vm_map.find(vm_id) == vm_map.end()){
			vm* v = new vm(vm_id, this, xs_path+"/"+x);
			v->update_vnode_map(ts);
			vm_map[vm_id] = v;	
		}
		else{
		// vm exists in vm_map
			vm_map[vm_id]->update_vnode_map(ts);
		}
	}
	
	// delete obsolete vm
	cout << UNIX_TS<< "Deleting obsolete vm" << endl;
	for(auto& x: vm_map){
		auto v = x.second;
		if(v->ts < ts){
			delete v;
			vm_map.erase(x.first);
		}
	}
	
	// update pnode info, after this function the pnode bw information is available.
	cout << UNIX_TS<< "Updating pnode_list" << endl;
	for(auto& x: pnode_list){
		x->update_vnode_map(ts);
	}
	
	// get log entries from xs
	for(auto& x: vm_map){
		if(x.first!=0){
			auto v = x.second;
			v->logger->crawl_log_entries_from_xs();
			//v->logger->insert_log_entry(time(0)*1000,"This is just a test");
			v->logger->flush_log_to_disk();
		}
	}

}

int topo_change_d::shrink_vm(int id, int vnode_id){
	vm* v = get_vm_by_id(id);
	if(!v){
		cout<< UNIX_TS<<"Didn't find vm " << id << " in topo_change_d::shrink_vm" <<endl;
		return -1;
	}
	return v->shrink_vnode(vnode_id);
}
int topo_change_d::expand_vm(int id, int vnode_id){
	vm* v = get_vm_by_id(id);
	if(!v){
		cout<< UNIX_TS<<"Didn't find vm " << id << " in topo_change_d::expand_vm" <<endl;
		return -1;
	}
	return v->expand_vnode(vnode_id);
}
        
void topo_change_d::register_pvnode(int vm_id, vnode* n, int pnode_id){
	if(pnode_id >= pnode_list.size()){
		while(pnode_id >= pnode_list.size()) {pnode_list.push_back(new pnode(pnode_list.size()));}
	}
	pnode_list[pnode_id]->register_vnode(vm_id, n);
}

void topo_change_d::unregister_vnode(int vm_id, vnode* n, int pnode_id){
	assert(pnode_id < pnode_list.size());
	pnode_list[pnode_id]->unregister_vnode(vm_id, n);
}

long topo_change_d::pnode_average_bw_usage(int pnode_id){
	assert(pnode_id < pnode_list.size());
	return pnode_list[pnode_id]->average_bw_usage();
}

int topo_change_d::pnode_num_active_vnode(int pnode_id){
	assert(pnode_id < pnode_list.size());
	return pnode_list[pnode_id]->active_vnodes;
}
long long topo_change_d::pnode_cpu_usage(int pnode_id){
	assert(pnode_id < pnode_list.size());
	return pnode_list[pnode_id]->recent_cpu_usage;
}

void topo_change_d::start(){
	while(!interrupted){
		while(!interrupted && event_list.empty()){
			usleep(interval_us);
			generate_events();
		}
		process_events();
	}
}

void topo_change_d::interrupt(){
	interrupted = true;
}

void topo_change_d::process_event(topo_change_event& e){
	if(e.action!= 1 && e.action!=-1){
		cout << UNIX_TS<< "Cannot process event on vm: " << e.vm_id <<" in topo_change_d::process_event"<< endl;
		return;
	}
	int (topo_change_d::*action_func)(int, int) = e.action==1? (&topo_change_d::expand_vm) : (&topo_change_d::shrink_vm);
	for(int node: e.vnode_list){
		(*this.*action_func)(e.vm_id, node);
	}
}

void topo_change_d::process_events(){
	while(!event_list.empty()){
		auto& x = event_list.front();
		process_event(x);
		event_list.pop_front();
	}
}
void topo_change_d::generate_events(){
	
   	struct sysinfo s_info;
    	int error = sysinfo(&s_info);
    	if(error != 0)
    	{
        	cout << UNIX_TS << "Error when getting system uptime, code error = "  << error << endl;
	}
	cout << UNIX_TS<< "==================topo_change_d::generate_events(), ts:" << ts <<"===== " <<s_info.uptime<< " ===========" << endl;
	if(file_output)
	of << UNIX_TS<< "==================topo_change_d::generate_events(), ts:" << ts <<"===== " <<s_info.uptime<< " ===========" << endl;
	update_vm_map();
	engine->generate_events(event_list);			
}
