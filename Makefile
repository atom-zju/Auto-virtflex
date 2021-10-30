CXX:=g++
LFLAG:=-lxenstore -lxenctrl -lxenlight -lxentoollog
#CFLAG:=-g -std=c++11
CFLAG:=-g -std=gnu++11
OBJ:= main.o cpu.o util.o topo_change_d.o vm.o node.o pnode.o vnode.o topo_change_engine.o vm_logger.o sample_queue.o sys_map.o workload_attr.o topo_sys_map_generator.o

all: topo_daemon

topo_daemon: $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LFLAG)  

%.o: %.cpp %.h
	$(CXX) -c $< $(CFLAG) -o $@

clean:
	rm *.o topo_daemon
