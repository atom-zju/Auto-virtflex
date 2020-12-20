CXX:=g++
LFLAG:=-lxenstore
CFLAG:=-g -std=c++11
OBJ:= main.o cpu.o topo_change_d.o vm.o node.o pnode.o vnode.o

all: topo_daemon

topo_daemon: $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LFLAG)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAG)

clean:
	rm *.o topo_daemon
