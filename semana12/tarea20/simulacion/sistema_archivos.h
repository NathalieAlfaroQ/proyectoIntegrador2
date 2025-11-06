#ifndef SISTEMA_ARCHIVOS_H
#define SISTEMA_ARCHIVOS_H

#include <string>
#include <vector>
#include <map>

class SistemaArchivos {
private:
    std::string directorioFiguras;
    std::map<std::string, std::string> figurasDisponibles;

public:
    SistemaArchivos(const std::string& dir);
    
    // Métodos principales
    std::string obtenerFigura(const std::string& nombre);
    std::vector<std::string> listarFiguras();
    bool agregarFigura(const std::string& nombre, const std::string& contenido);
    bool existeFigura(const std::string& nombre);
};

#endif