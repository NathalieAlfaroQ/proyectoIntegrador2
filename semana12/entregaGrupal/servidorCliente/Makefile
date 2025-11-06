# Makefile limpio y funcional

CXX = g++
CXXFLAGS = -Wall -g

# Ejecutables
TARGETS = figureServer.out client.out cacheServer.out

all: $(TARGETS)

# =======================
# Servidor HTTP principal
# =======================
figureServer.out: figureServer.cc Socket.cc VSocket.cc FileSystem.cc Superblock.cc Cache.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

# =======================
# Servidor caché
# =======================
cacheServer.out: cacheServer.cc Socket.cc VSocket.cc FileSystem.cc Superblock.cc Cache.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

# =======================
# Cliente HTTP
# =======================
client.out: client.cc Socket.cc VSocket.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

# =======================
# Limpieza
# =======================
clean:
	rm -f *.o $(TARGETS) FileSystem.bin
