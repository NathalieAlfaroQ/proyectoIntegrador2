# Makefile limpio y optimizado
CXX = g++
CC = gcc
CXXFLAGS = -Iinclude -Wall -g -Wextra 
CFLAGS = -Iinclude -Wall -Wextra
LDFLAGS = -lssl -lcrypto

# Ejecutables
TARGETS =  HTTPServer.out  HTTPClient.out

all: $(TARGETS)

HTTPServer.out: HTTPServer.cc SSLSocket.cc VSocket.cc FileSystem.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

HTTPClient.out: HTTPClient.cc SSLSocket.cc VSocket.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o $(TARGETS) FileSystem.bin