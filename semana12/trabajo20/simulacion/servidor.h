#ifndef SERVIDOR_H
#define SERVIDOR_H

#include "sistema_archivos.h"
#include "cache.h"
#include <string>

class Servidor {
private:
    SistemaArchivos sistemaArchivos;
    Cache cache;

public:
    Servidor();
    
    // Métodos principales
    std::string solicitarFigura(const std::string& nombre);
    std::vector<std::string> listarFiguras();
    bool agregarFigura(const std::string& nombre, const std::string& contenido);
    void mostrarEstadisticas();
};

#endif