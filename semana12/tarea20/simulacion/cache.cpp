#include "cache.h"
#include <iostream>

Cache::Cache() : hits(0), misses(0) {
    std::cout << "CACHE: Inicializando caché de figuras" << std::endl;
}

std::string Cache::obtenerFigura(const std::string& nombre) {
    std::cout << "CACHE: Buscando figura '" << nombre << "' en caché" << std::endl;
    
    auto it = cacheFiguras.find(nombre);
    if (it != cacheFiguras.end()) {
        hits++;
        std::cout << "CACHE: HIT - Figura '" << nombre << "' encontrada en caché" << std::endl;
        return it->second;
    }
    
    misses++;
    std::cout << "CACHE: MISS - Figura '" << nombre << "' NO encontrada en caché" << std::endl;
    return "";
}

void Cache::guardarFigura(const std::string& nombre, const std::string& contenido) {
    std::cout << "CACHE: Guardando figura '" << nombre << "' en caché" << std::endl;
    cacheFiguras[nombre] = contenido;
    std::cout << "CACHE: Figura '" << nombre << "' guardada exitosamente" << std::endl;
}

bool Cache::tieneFigura(const std::string& nombre) {
    return cacheFiguras.find(nombre) != cacheFiguras.end();
}

void Cache::mostrarEstadisticas() {
    std::cout << "\n ESTADÍSTICAS DE CACHE:" << std::endl;
    std::cout << "   Hits: " << hits << std::endl;
    std::cout << "   Misses: " << misses << std::endl;
    std::cout << "   Total de solicitudes: " << (hits + misses) << std::endl;
    std::cout << "   Tasa de aciertos: " << (hits * 100.0 / (hits + misses)) << "%" << std::endl;
    std::cout << "   Figuras en caché: " << cacheFiguras.size() << std::endl;
}