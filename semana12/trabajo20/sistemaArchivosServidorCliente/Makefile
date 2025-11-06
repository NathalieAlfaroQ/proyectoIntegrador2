# Makefile limpio y optimizado

CXX = g++
CXXFLAGS = -Wall -g

# Ejecutables
TARGETS = HTTPServer.out HTTPClient.out CacheServer.out

all: $(TARGETS)



HTTPServer.out: HTTPServer.cc Socket.cc VSocket.cc FileSystem.cc
	$(CXX) $(CXXFLAGS) -o $@ $^



HTTPClient.out: HTTPClient.cc Socket.cc VSocket.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

CacheServer.out: cache.cc Socket.cc VSocket.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGETS) FileSystem.bin
