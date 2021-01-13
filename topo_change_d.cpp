#include <iostream>
#include <unistd.h>
#include <cassert>
#include "topo_change_d.h"
#include "util.h"
#include "topo_change_engine.h"

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

topo_change_d::topo_change_d(){
	ts = 0;
	xs =  xs_daemon_open();
        if ( xs == NULL ) {
                perror("Error when connecting to a xs daemon\n");
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
	delete engine;
}

void topo_change_d::update_vm_map(){
	ts++;
	vector<string> dir;
	list_xenstore_directory(xs, string("/local/domain"), dir);
	cout<<"/local/domain"<< endl;
	for(auto& x: dir){
		int vm_id =  stoi(x);
		cout <<"\t" <<"VM: "<<stoi(x) << endl;
		if(vm_map.find(vm_id) == vm_map.end()){
			vm* v = new vm(vm_id, this, string("/local/domain/").append(x));
			v->update_vnode_map(ts);
			vm_map[vm_id] = v;	
		}
		else{
		// vm exists in vm_map
			vm_map[vm_id]->update_vnode_map(ts);
		}
	}
	
	// delete obsolete vm
	for(auto& x: vm_map){
		auto v = x.second;
		if(v->ts < ts){
			delete v;
			vm_map.erase(x.first);
		}
	}

	for(auto& x: pnode_list){
		x->update_vnode_map(ts);
	}

}

int topo_change_d::shrink_vm(int id, int vnode_id){
	vm* v = get_vm_by_id(id);
	if(!v){
		cout<<"Didn't find vm " << id << " in topo_change_d::shrink_vm" <<endl;
		return -1;
	}
	return v->shrink_vnode(vnode_id);
}
int topo_change_d::expand_vm(int id, int vnode_id){
	vm* v = get_vm_by_id(id);
	if(!v){
		cout<<"Didn't find vm " << id << " in topo_change_d::expand_vm" <<endl;
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

void topo_change_d::start(){
	while(1){
		while(event_list.empty()){
			usleep(interval_us);
			generate_events();
		}
		process_events();
	}

}

void topo_change_d::process_event(topo_change_event& e){
	if(e.action!= 1 && e.action!=-1){
		cout << "Cannot process event on vm: " << e.vm_id <<" in topo_change_d::process_event"<< endl;
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
	cout << "==================topo_change_d::generate_events(), ts:" << ts <<"================" << endl;
	update_vm_map();
	engine->generate_events(event_list);			
}
