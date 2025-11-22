#include <iostream>
#include <fstream>
#include <cstring>
#include <limits>
#include "FileSystem.h"

FileSystem::FileSystem(const std::string& archivo) : archivoBin(archivo) {}

// Función para leer completamente un archivo de texto como string
std::string FileSystem::Leer_Archivo(const std::string& nombre_archivo) {
    // Apertura del archivo para lectura/escritura binaria
    std::ifstream archivo(nombre_archivo);
    if (!archivo.is_open()) {
    // Mensaje de error si no se pudo abrir el archivo
        std::cerr << "[ERROR] No se pudo abrir " << nombre_archivo << "\n";
    // Retorno de valor (string vacío o contenido del archivo)
        return "";
    }
    // Retorno de valor (string vacío o contenido del archivo)
    return std::string((std::istreambuf_iterator<char>(archivo)), std::istreambuf_iterator<char>());
}

// Función para crear un archivo binario que simula el sistema de archivos
void FileSystem::Crear_FileSystem() {
    // Apertura del archivo para lectura/escritura binaria
    std::ofstream archivo(archivoBin, std::ios::binary | std::ios::trunc);
    // Se crea un bloque de tamaño fijo con ceros
    std::vector<char> bloque0(TAM_BLOQUE, 0);
    uint32_t primer_directorio = 1;
    // Copia el índice del primer directorio al inicio del bloque
    std::memcpy(&bloque0[0], &primer_directorio, 4);
    // Escribe el bloque en el archivo binario
    archivo.write(bloque0.data(), TAM_BLOQUE);

    // Se crea un bloque de tamaño fijo con ceros
    std::vector<char> dir(TAM_BLOQUE, 0);
    // Escribe el bloque en el archivo binario
    archivo.write(dir.data(), TAM_BLOQUE);
    archivo.close();

    std::cout << "[INFO] Sistema de archivos creado con el directorio inicial en bloque 1.\n";
}

std::set<uint32_t> FileSystem::Obtener_Bloques_Ocupados() {
    // Apertura del archivo para lectura/escritura binaria
    std::ifstream archivo(archivoBin, std::ios::binary);
    std::set<uint32_t> ocupados;

    archivo.seekg(0, std::ios::end);
    archivo.seekg(0);

    // Se crea un bloque de tamaño fijo con ceros
    std::vector<char> bloque0(TAM_BLOQUE);
    archivo.read(bloque0.data(), TAM_BLOQUE);

    for (int i = 0; i < TAM_BLOQUE; i += 4) {
        uint32_t dir_block;
    // Copia el índice del primer directorio al inicio del bloque
        std::memcpy(&dir_block, &bloque0[i], 4);
        if (dir_block == 0) break;
        ocupados.insert(dir_block);
        archivo.seekg(dir_block * TAM_BLOQUE);
    // Se crea un bloque de tamaño fijo con ceros
        std::vector<char> dir(TAM_BLOQUE);
        archivo.read(dir.data(), TAM_BLOQUE);
        for (int j = 0; j < ENTRADAS_POR_DIRECTORIO; ++j) {
            int offset = j * TAM_ENTRADA;
            if (dir[offset] == 0) continue;
    // Índice del primer bloque del archivo
            uint32_t inicio;
            uint16_t cant;
    // Copia el índice del primer directorio al inicio del bloque
            std::memcpy(&inicio, &dir[offset + 14], 4);
    // Copia el índice del primer directorio al inicio del bloque
            std::memcpy(&cant, &dir[offset + 18], 2);
            for (uint16_t k = 0; k < cant; ++k) ocupados.insert(inicio + k);
        }
    }

    // Retorno de valor (string vacío o contenido del archivo)
    return ocupados;
}

    // Se crea un bloque de tamaño fijo con ceros
std::uint32_t FileSystem::Crear_Directorio(std::fstream& archivo_bin, std::vector<char>& bloque0) {
    archivo_bin.seekg(0, std::ios::end);
    uint32_t nuevo_bloque = archivo_bin.tellg() / TAM_BLOQUE;
    // Se crea un bloque de tamaño fijo con ceros
    std::vector<char> nuevo_dir(TAM_BLOQUE, 0);
    archivo_bin.write(nuevo_dir.data(), TAM_BLOQUE);
    for (int i = 0; i < TAM_BLOQUE; i += 4) {
        uint32_t ptr;
    // Copia el índice del primer directorio al inicio del bloque
        std::memcpy(&ptr, &bloque0[i], 4);
        if (ptr == 0) {
    // Copia el índice del primer directorio al inicio del bloque
            std::memcpy(&bloque0[i], &nuevo_bloque, 4);
            std::cout << "Directorio creado en el bloque " << nuevo_bloque << "\n";
    // Retorno de valor (string vacío o contenido del archivo)
            return nuevo_bloque;
        }
    }
    // Retorno de valor (string vacío o contenido del archivo)
    return nuevo_bloque;
}

    // Se crea un bloque de tamaño fijo con ceros
std::uint32_t FileSystem::Encontrar_Crear_Directorio(std::fstream& archivo_bin, std::vector<char>& bloque0) {
    for (int i = 0; i < TAM_BLOQUE; i += 4) {
        uint32_t dir_block;
    // Copia el índice del primer directorio al inicio del bloque
        std::memcpy(&dir_block, &bloque0[i], 4);
        if (dir_block == 0) break;
        archivo_bin.seekg(dir_block * TAM_BLOQUE);
    // Se crea un bloque de tamaño fijo con ceros
        std::vector<char> dir(TAM_BLOQUE);
        archivo_bin.read(dir.data(), TAM_BLOQUE);
        for (int j = 0; j < ENTRADAS_POR_DIRECTORIO; ++j) {
    // Retorno de valor (string vacío o contenido del archivo)
            if (dir[j * TAM_ENTRADA] == 0) return dir_block;
        }
    }
    // Retorno de valor (string vacío o contenido del archivo)
    return Crear_Directorio(archivo_bin, bloque0);
}

bool  FileSystem::Agregar_Figura(const std::string& nombre_archivo) {
    // Se construye la ruta completa con prefijo y sufijo
    std::string ruta_archivo = "Figuras/" + nombre_archivo;
    if (ruta_archivo.find(".txt") == std::string::npos) {
        ruta_archivo += ".txt";
    }
    std::fstream archivo_bin(archivoBin, std::ios::in | std::ios::out | std::ios::binary);
    if (!archivo_bin.is_open()) {
    // Mensaje de error si no se pudo abrir el archivo
        std::cerr << "[ERROR] No se pudo abrir el archivo.\n";
    // Retorno de valor (string vacío o contenido del archivo)
        return false;
    }

    std::string figura = Leer_Archivo(ruta_archivo);
    // Retorno de valor (string vacío o contenido del archivo)
    if (figura.empty()) return false;

    size_t slash = ruta_archivo.find_last_of("/\\");
    std::string nombre_figura = ruta_archivo.substr(slash == std::string::npos ? 0 : slash + 1);

    if (nombre_figura.find(".txt") == std::string::npos) {
        nombre_figura += ".txt";
    }

    // Se crea un bloque de tamaño fijo con ceros
    std::vector<char> bytes(figura.begin(), figura.end());
    size_t bloques_necesarios = (bytes.size() + TAM_BLOQUE - 1) / TAM_BLOQUE;

    std::set<uint32_t> ocupados = Obtener_Bloques_Ocupados();
    archivo_bin.seekg(0, std::ios::end);
    uint32_t siguiente = archivo_bin.tellg() / TAM_BLOQUE;

    std::vector<uint32_t> bloques_usados;
    for (uint32_t i = 2; bloques_usados.size() < bloques_necesarios; ++i) {
        if (ocupados.find(i) == ocupados.end()) {
            bloques_usados.push_back(i);
            if (i >= siguiente) {
                archivo_bin.seekp((i + 1) * TAM_BLOQUE - 1);
                archivo_bin.write("", 1);
            }
        }
    }

    for (size_t i = 0; i < bloques_usados.size(); ++i) {
        archivo_bin.seekp(bloques_usados[i] * TAM_BLOQUE);
        archivo_bin.write(&bytes[i * TAM_BLOQUE], std::min((size_t)TAM_BLOQUE, bytes.size() - i * TAM_BLOQUE));
    }

    archivo_bin.seekg(0);
    // Se crea un bloque de tamaño fijo con ceros
    std::vector<char> bloque0(TAM_BLOQUE);
    archivo_bin.read(bloque0.data(), TAM_BLOQUE);

    uint32_t dir_block = Encontrar_Crear_Directorio(archivo_bin, bloque0);
    archivo_bin.seekg(dir_block * TAM_BLOQUE);
    // Se crea un bloque de tamaño fijo con ceros
    std::vector<char> dir(TAM_BLOQUE);
    archivo_bin.read(dir.data(), TAM_BLOQUE);

    for (int j = 0; j < ENTRADAS_POR_DIRECTORIO; ++j) {
        int offset = j * TAM_ENTRADA;
        if (dir[offset] == 0) {
    // Nombre del archivo (13 caracteres + terminador nulo)
            char nombre[14] = {0};
    // Copia el índice del primer directorio al inicio del bloque
            std::memcpy(nombre, nombre_figura.c_str(), std::min(size_t(14), nombre_figura.size()));
    // Copia el índice del primer directorio al inicio del bloque
            std::memcpy(&dir[offset], nombre, 14);
    // Índice del primer bloque del archivo
            uint32_t inicio = bloques_usados[0];
            uint16_t cant = bloques_usados.size();
    // Copia el índice del primer directorio al inicio del bloque
            std::memcpy(&dir[offset + 14], &inicio, 4);
    // Copia el índice del primer directorio al inicio del bloque
            std::memcpy(&dir[offset + 18], &cant, 2);
            break;
        }
    }

    archivo_bin.seekp(dir_block * TAM_BLOQUE);
    archivo_bin.write(dir.data(), TAM_BLOQUE);
    archivo_bin.seekp(0);
    archivo_bin.write(bloque0.data(), TAM_BLOQUE);

    archivo_bin.close();

    /*std::cout << "[INFO] Figura '" << nombre_figura << "' agregada con éxito en bloque(s): ";
    for (size_t i = 0; i < bloques_usados.size(); ++i) {
        std::cout << bloques_usados[i];
        if (i + 1 < bloques_usados.size()) std::cout << ", ";
    }
    std::cout << "\n";*/

    return true;
}

// Función que busca una figura por nombre y la retorna como un string
std::string FileSystem::Leer_Figura(const std::string& nombre_buscar) {
    std::ifstream archivo(archivoBin, std::ios::binary);
    if (!archivo.is_open()) {
        std::cerr << "[ERROR] No se pudo abrir el archivo.\n";
        return "";
    }

    // Leer el bloque 0 que contiene los punteros a los bloques de directorio
    std::vector<char> bloque0(TAM_BLOQUE);
    archivo.read(bloque0.data(), TAM_BLOQUE);

    // Iterar por cada bloque de directorio listado en el bloque 0
    for (int i = 0; i < TAM_BLOQUE; i += 4) {
        uint32_t dir_block;
        std::memcpy(&dir_block, &bloque0[i], 4);
        if (dir_block == 0) break;  // Fin de la lista

        // Leer el bloque de directorio actual
        archivo.seekg(dir_block * TAM_BLOQUE);
        std::vector<char> dir(TAM_BLOQUE);
        archivo.read(dir.data(), TAM_BLOQUE);

        // Buscar la figura dentro de este bloque de directorio
        for (int j = 0; j < ENTRADAS_POR_DIRECTORIO; ++j) {
            int offset = j * TAM_ENTRADA;
            char nombre[15] = {0};
            std::memcpy(nombre, &dir[offset], 14);

            // Comparar el nombre buscado con el actual
            if (nombre_buscar == nombre) {
                // Obtener información de la figura: bloque inicial y cantidad de bloques
                uint32_t inicio;
                uint16_t bloques;
                std::memcpy(&inicio, &dir[offset + 14], 4);
                std::memcpy(&bloques, &dir[offset + 18], 2);

                // Leer todos los bloques asociados a la figura
                std::string figura;
                for (int k = 0; k < bloques; ++k) {
                    archivo.seekg((inicio + k) * TAM_BLOQUE);
                    std::vector<char> datos(TAM_BLOQUE);
                    archivo.read(datos.data(), TAM_BLOQUE);
                    figura.append(datos.data(), TAM_BLOQUE);
                }

                return figura;  // Retornar el contenido de la figura
            }
        }
    }

    // Si no se encontró, retornar mensaje de no encontrado
    return "[INFO] Figura '" + nombre_buscar + "' no encontrada.\n";
}


// Función que devuelve un string con todas las figuras disponibles en el sistema de archivos
std::string FileSystem::Listar_Figuras() {
    std::ifstream archivo(archivoBin, std::ios::binary);
    if (!archivo.is_open()) {
        return "[ERROR] No se pudo abrir el archivo.\n";
    }

    std::string resultado = "[INFO] Figuras almacenadas en el sistema:\n";

    // Leer el bloque 0 que contiene los punteros a bloques de directorio
    std::vector<char> bloque0(TAM_BLOQUE);
    archivo.read(bloque0.data(), TAM_BLOQUE);

    // Recorrer todos los bloques de directorio listados en el bloque 0
    for (int i = 0; i < TAM_BLOQUE; i += 4) {
        uint32_t dir_block;
        std::memcpy(&dir_block, &bloque0[i], 4);
        if (dir_block == 0) break;

        // Leer el contenido del bloque de directorio
        archivo.seekg(dir_block * TAM_BLOQUE);
        std::vector<char> dir(TAM_BLOQUE);
        archivo.read(dir.data(), TAM_BLOQUE);

        // Iterar sobre todas las entradas del directorio y mostrar los nombres válidos
        for (int j = 0; j < ENTRADAS_POR_DIRECTORIO; ++j) {
            int offset = j * TAM_ENTRADA;
            if (dir[offset] != 0) {  // Si hay una entrada válida
                char nombre[15] = {0};
                std::memcpy(nombre, &dir[offset], 14);
                resultado += "- " + std::string(nombre) + "\n";
            }
        }
    }

    return resultado;
}
