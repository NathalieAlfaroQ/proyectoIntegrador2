# Makefile para archivos .cc

CXX = g++
CXXFLAGS = -Wall -g

# Ejecutables
TARGETS = HTTPServer.out HTTPClient.out cache.out 

all: $(TARGETS)

# Servidor HTTP principal
HTTPServer.out: HTTPServer.cc Socket.cc VSocket.cc FileSystem.cc Utils.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

# Cliente HTTP
HTTPClient.out: HTTPClient.cc Socket.cc VSocket.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

# Servidor de Cache
cache.out: cache.cc Socket.cc VSocket.cc Utils.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

# Nuevo Intermediario
# Intermediario.out: Intermediario.cc Socket.cc VSocket.cc

clean:
	rm -f *.o $(TARGETS) filesystem.bin

.PHONY: all clean