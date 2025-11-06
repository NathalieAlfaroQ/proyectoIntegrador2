CXX = g++
CXXFLAGS = -std=c++17 -Wall
TARGET = sistema_figuras
OBJS = cliente.o servidor.o cache.o sistema_archivos.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

cliente.o: cliente.cpp servidor.h
	$(CXX) $(CXXFLAGS) -c cliente.cpp

servidor.o: servidor.cpp servidor.h sistema_archivos.h cache.h
	$(CXX) $(CXXFLAGS) -c servidor.cpp

cache.o: cache.cpp cache.h
	$(CXX) $(CXXFLAGS) -c cache.cpp

sistema_archivos.o: sistema_archivos.cpp sistema_archivos.h
	$(CXX) $(CXXFLAGS) -c sistema_archivos.cpp

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET)

.PHONY: clean run