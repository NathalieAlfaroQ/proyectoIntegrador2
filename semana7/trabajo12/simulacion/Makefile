# Nombre del compilador
CXX = g++

# Opciones de compilación
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread

# Archivos fuente
SOURCES = Servidor.cc Tenedor.cc Cliente.cc main.cc

# Archivos objeto
OBJECTS = $(SOURCES:.cc=.o)

# Nombre del ejecutable
TARGET = simulacion

# Regla para compilar el ejecutable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Regla para compilar archivos objeto
.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGET) $(OBJECTS)