# Makefile limpio y optimizado

CXX = g++
CXXFLAGS = -Wall -g

# Ejecutables
TARGETS = Server.out HTTPServer.out Client.out HTTPClient.out

all: $(TARGETS)

Server.out: Server.cc Socket.cc VSocket.cc FileSystem.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

HTTPServer.out: HTTPServer.cc Socket.cc VSocket.cc FileSystem.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

Client.out: Client.cc Socket.cc VSocket.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

HTTPClient.out: HTTPClient.cc Socket.cc VSocket.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGETS) FileSystem.bin
