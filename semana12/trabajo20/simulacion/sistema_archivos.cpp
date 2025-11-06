#include "sistema_archivos.h"
#include <iostream>
#include <fstream>
#include <filesystem>

SistemaArchivos::SistemaArchivos(const std::string& dir) : directorioFiguras(dir) {
    
    // Crear directorio si no existe
    std::filesystem::create_directories(dir);
    
    // Cargar figuras disponibles
    figurasDisponibles = {
        {"gato", "figuras/gato.txt"},
        {"casa", "figuras/casa.txt"},
        {"arbol", "figuras/arbol.txt"}
    };
    
    // Crear archivos de ejemplo
    std::ofstream file1("figuras/gato.txt");
    file1 << " /\\_/\\ \n( o.o )\n > ^ < \n";
    file1.close();
    
    std::ofstream file2("figuras/casa.txt");
    file2 << "  /\\ \n /  \\ \n/____\\ \n|    |\n|____|\n";
    file2.close();
    
    std::ofstream file3("figuras/arbol.txt");
    file3 << "   * \n  *** \n ***** \n*******\n   | \n   | \n";
    file3.close();
    
    std::cout << "SISTEMA_ARCHIVOS: Sistema inicializado con " << figurasDisponibles.size() << " figuras" << std::endl;
}

std::string SistemaArchivos::obtenerFigura(const std::string& nombre) {
    std::cout << " SISTEMA_ARCHIVOS: Solicitando figura '" << nombre << "'" << std::endl;
    
    if (figurasDisponibles.find(nombre) == figurasDisponibles.end()) {
        std::cout << " SISTEMA_ARCHIVOS: Figura '" << nombre << "' no encontrada" << std::endl;
        return "";
    }
    
    std::ifstream archivo(figurasDisponibles[nombre]);
    if (!archivo.is_open()) {
        std::cout << " SISTEMA_ARCHIVOS: Error al abrir archivo para '" << nombre << "'" << std::endl;
        return "";
    }
    
    std::string contenido, linea;
    while (std::getline(archivo, linea)) {
        contenido += linea + "\n";
    }
    archivo.close();
    
    std::cout << " SISTEMA_ARCHIVOS: Figura '" << nombre << "' obtenida exitosamente" << std::endl;
    return contenido;
}

std::vector<std::string> SistemaArchivos::listarFiguras() {
    std::vector<std::string> lista;
    for (const auto& figura : figurasDisponibles) {
        lista.push_back(figura.first);
    }
    std::cout << " SISTEMA_ARCHIVOS: Listando " << lista.size() << " figuras disponibles" << std::endl;
    return lista;
}

bool SistemaArchivos::agregarFigura(const std::string& nombre, const std::string& contenido) {
    std::cout << " SISTEMA_ARCHIVOS: Intentando agregar nueva figura '" << nombre << "'" << std::endl;
    
    if (figurasDisponibles.find(nombre) != figurasDisponibles.end()) {
        std::cout << " SISTEMA_ARCHIVOS: La figura '" << nombre << "' ya existe" << std::endl;
        return false;
    }
    
    std::string rutaArchivo = directorioFiguras + "/" + nombre + ".txt";
    std::ofstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        std::cout << " SISTEMA_ARCHIVOS: Error al crear archivo para '" << nombre << "'" << std::endl;
        return false;
    }
    
    archivo << contenido;
    archivo.close();
    
    figurasDisponibles[nombre] = rutaArchivo;
    std::cout << " SISTEMA_ARCHIVOS: Nueva figura '" << nombre << "' agregada exitosamente" << std::endl;
    return true;
}

bool SistemaArchivos::existeFigura(const std::string& nombre) {
    return figurasDisponibles.find(nombre) != figurasDisponibles.end();
}