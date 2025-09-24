# Compiladores
CXX = g++
CC = gcc

# Flags de compilación
CXXFLAGS = -Iinclude -Wall -Wextra
CFLAGS = -Iinclude -Wall -Wextra

# Librerías de enlace
LDFLAGS = -lssl -lcrypto

# Objetos comunes
OBJS = VSocket.o SSLSocket.o

# Ejecutables
TARGETS = SSLServer SSLClient

all: $(TARGETS)

SSLServer: SSLServer.o $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

SSLClient: SSLClient.o $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Regla genérica para .o
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpieza
clean:
	rm -f *.o $(TARGETS)
