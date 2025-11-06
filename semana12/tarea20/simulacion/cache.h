#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <map>

class Cache {
private:
    std::map<std::string, std::string> cacheFiguras;
    int hits;
    int misses;

public:
    Cache();
    
    // Métodos principales
    std::string obtenerFigura(const std::string& nombre);
    void guardarFigura(const std::string& nombre, const std::string& contenido);
    bool tieneFigura(const std::string& nombre);
    void mostrarEstadisticas();
};

#endif