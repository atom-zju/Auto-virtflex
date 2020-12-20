#include "vm.h"

using namespace std;

vm::~vm(){
	for(auto& x: vnode_map){
		delete x.second;
	}
}
