# Linux Makefile 

CC  := gcc
CXX := g++
CFLAGS := -g -c
CXXFLAGS := -g -c -std=c++11
LDFLAGS := -pthread
DEPS := Socket.h lector_figura.h
TARGETS := MirrorClient.out ThreadMirrorServer.out

ALL := $(TARGETS)
$(info $$TARGETS = $(TARGETS))

%.o: %.cc $(DEPS)
	$(CXX) $(CXXFLAGS) $<

%.out: %.o VSocket.o Socket.o lector_figura.o
	$(info $$^ = $^)
	$(CXX) $^ $(LDFLAGS) -o $@
	
ThreadMirrorServer.out:	ThreadMirrorServer.o VSocket.o Socket.o lector_figura.o
	$(CXX) ThreadMirrorServer.o VSocket.o Socket.o lector_figura.o $(LDFLAGS) -o $@

lector_figura.o: lector_figura.cc lector_figura.h
	$(CXX) $(CXXFLAGS) lector_figura.cc -o lector_figura.o
	
clean:
	rm -f *.o *.out