#include "servidor.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>  

void mostrarMenu() {
    std::cout << "1. Solicitar figura" << std::endl;
    std::cout << "2. Ver figuras disponibles" << std::endl;
    std::cout << "3. Agregar nueva figura" << std::endl;
    std::cout << "Seleccione una opción: ";
}

std::string leerArchivoTxt() {
    std::string nombreArchivo;
    std::cout << "Ingrese la ruta del archivo .txt: ";
    std::getline(std::cin, nombreArchivo); 
    
    std::ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        std::cout << "Error: No se pudo abrir el archivo " << nombreArchivo << std::endl;
        return "";
    }
    
    std::string contenido, linea;
    while (std::getline(archivo, linea)) {
        contenido += linea + "\n";
    }
    archivo.close();
    
    std::cout << "Archivo leído exitosamente (" << contenido.size() << " caracteres)" << std::endl;
    return contenido;
}

int main() {
    std::cout << "CLIENTE: Iniciando cliente..." << std::endl;
    Servidor servidor;
    
    int opcion;
    std::string nombreFigura;
    
    do {
        mostrarMenu();
        std::cin >> opcion;
        std::cin.ignore(); 
        
        switch (opcion) {
            case 1: {
                std::cout << "Ingrese el nombre de la figura: ";
                std::getline(std::cin, nombreFigura);
                
                std::cout << "\nCLIENTE: Enviando solicitud al servidor..." << std::endl;
                std::string figura = servidor.solicitarFigura(nombreFigura);
                
                std::cout << "\nFIGURA SOLICITADA:\n" << figura << std::endl;
                break;
            }
            
            case 2: {
                std::cout << "\nCLIENTE: Solicitando listado de figuras..." << std::endl;
                std::vector<std::string> figuras = servidor.listarFiguras();
                
                std::cout << "\nFIGURAS DISPONIBLES:" << std::endl;
                for (const auto& figura : figuras) {
                    std::cout << "   • " << figura << std::endl;
                }
                break;
            }
            
            case 3: {
                std::cout << "➕ Ingrese el nombre para la nueva figura: ";
                std::getline(std::cin, nombreFigura);
                
                std::string contenido = leerArchivoTxt();
                if (!contenido.empty()) {
                    std::cout << "\nCLIENTE: Enviando nueva figura al servidor..." << std::endl;
                    if (servidor.agregarFigura(nombreFigura, contenido)) {
                        std::cout << "Figura '" << nombreFigura << "' agregada exitosamente" << std::endl;
                    } else {
                        std::cout << "Error al agregar la figura" << std::endl;
                    }
                }
                break;
            }
            
            default: {
                std::cout << "Opción inválida" << std::endl;
                break;
            }
        }
        
    } while (opcion != 5);
    
    return 0;
}