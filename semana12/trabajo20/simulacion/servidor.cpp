#include "servidor.h"
#include <iostream>

Servidor::Servidor() : sistemaArchivos("figuras"), cache() {
    std::cout << " SERVIDOR: Inicializando servidor..." << std::endl;
}

std::string Servidor::solicitarFigura(const std::string& nombre) {
    std::cout << "\nSERVIDOR: Procesando solicitud para figura '" << nombre << "'" << std::endl;
    
    // Paso 1: Consultar a la caché
    std::cout << "  SERVIDOR: Consultando a la caché..." << std::endl;
    std::string figura = cache.obtenerFigura(nombre);
    
    if (!figura.empty()) {
        std::cout << " SERVIDOR: Figura obtenida de la caché, enviando al cliente" << std::endl;
        return figura;
    }
    
    // Paso 2: Si no está en caché, obtener del sistema de archivos
    std::cout << "SERVIDOR: Figura no encontrada en caché, consultando sistema de archivos..." << std::endl;
    figura = sistemaArchivos.obtenerFigura(nombre);
    
    if (figura.empty()) {
        std::cout << "SERVIDOR: Figura '" << nombre << "' no encontrada en el sistema" << std::endl;
        return "ERROR: Figura no encontrada";
    }
    
    // Paso 3: Guardar en caché para futuras solicitudes
    std::cout << "SERVIDOR: Guardando figura en caché para futuras solicitudes..." << std::endl;
    cache.guardarFigura(nombre, figura);
    
    std::cout << "SERVIDOR: Figura enviada al cliente y almacenada en caché" << std::endl;
    return figura;
}

std::vector<std::string> Servidor::listarFiguras() {
    std::cout << "SERVIDOR: Solicitando listado de figuras disponibles" << std::endl;
    return sistemaArchivos.listarFiguras();
}

bool Servidor::agregarFigura(const std::string& nombre, const std::string& contenido) {
    std::cout << "SERVIDOR: Procesando solicitud para agregar nueva figura '" << nombre << "'" << std::endl;
    
    bool resultado = sistemaArchivos.agregarFigura(nombre, contenido);
    if (resultado) {
        std::cout << "SERVIDOR: Nueva figura agregada exitosamente al sistema" << std::endl;
    } else {
        std::cout << "SERVIDOR: Error al agregar nueva figura" << std::endl;
    }
    
    return resultado;
}

void Servidor::mostrarEstadisticas() {
    std::cout << "\nSERVIDOR: Mostrando estadísticas del sistema:" << std::endl;
    cache.mostrarEstadisticas();
}