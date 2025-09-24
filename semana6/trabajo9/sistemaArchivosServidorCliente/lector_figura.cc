/*
Sistema de archivos sencillo para figuras ASCII
Bloques de 256 bytes, primer bloque es directorio
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <algorithm>

const int TAM_BLOQUE = 256;
const std::string ARCHIVO = "almacenamiento.bin";

// Leer figura del filesystem
std::string leer_figura(const std::string &nombre_figura)
{
    std::ifstream archivo(ARCHIVO, std::ios::binary);
    if (!archivo) return "No se pudo abrir el archivo binario.\n";

    std::vector<char> directorio(TAM_BLOQUE);
    archivo.read(directorio.data(), TAM_BLOQUE);

    uint8_t contador = static_cast<uint8_t>(directorio[TAM_BLOQUE - 1]);

    for (int i = 0; i < contador; ++i)
    {
        int offset = 2 + i * 16;
        char nombre_raw[15] = {0};
        std::memcpy(nombre_raw, &directorio[offset], 14);
        std::string nombre(nombre_raw);
        nombre.erase(nombre.find_last_not_of("\0 ") + 1);

        uint8_t inicio = static_cast<uint8_t>(directorio[offset + 14]);
        uint8_t final = static_cast<uint8_t>(directorio[offset + 15]);

        if (nombre == nombre_figura)
        {
            int byte_inicio = TAM_BLOQUE + (inicio - 1) * TAM_BLOQUE + 1;
            int byte_final  = TAM_BLOQUE + final * TAM_BLOQUE;
            int longitud = byte_final - byte_inicio;

            archivo.seekg(byte_inicio, std::ios::beg);
            std::vector<char> figura_bytes(longitud);
            archivo.read(figura_bytes.data(), longitud);

            std::string figura(figura_bytes.data(), longitud);
            figura.erase(figura.find_last_not_of('\0') + 1);

            return figura;
        }
    }

    return "Figura '" + nombre_figura + "' no encontrada.\n";
}

// Agregar figura nueva sin pisar bloques existentes
std::string agregar_figura(const std::string &nombre_figura, const std::string &contenido)
{
    std::fstream archivo(ARCHIVO, std::ios::in | std::ios::out | std::ios::binary);
    if (!archivo) return "No se pudo abrir el archivo binario.\n";

    // Leer directorio
    std::vector<char> directorio(TAM_BLOQUE);
    archivo.read(directorio.data(), TAM_BLOQUE);

    uint8_t contador = static_cast<uint8_t>(directorio[TAM_BLOQUE - 1]);

    // Verificar si ya existe
    for (int i = 0; i < contador; ++i)
    {
        int offset = 2 + i * 16;
        char nombre_raw[15] = {0};
        std::memcpy(nombre_raw, &directorio[offset], 14);
        std::string existente(nombre_raw);
        existente.erase(existente.find_last_not_of("\0 ") + 1);
        if (existente == nombre_figura) return "La figura ya existe.\n";
    }

    // Buscar primer bloque libre (después del directorio)
    int nuevo_inicio = 1; // bloque 1 es directorio
    bool bloque_encontrado = false;
    while (!bloque_encontrado)
    {
        bool ocupado = false;
        for (int i = 0; i < contador; ++i)
        {
            int offset = 2 + i*16;
            uint8_t inicio = static_cast<uint8_t>(directorio[offset+14]);
            uint8_t final  = static_cast<uint8_t>(directorio[offset+15]);
            if (nuevo_inicio >= inicio && nuevo_inicio <= final)
            {
                ocupado = true;
                break;
            }
        }
        if (!ocupado) bloque_encontrado = true;
        else nuevo_inicio++;
    }

    int nuevo_final = nuevo_inicio;

    // Escribir en directorio
    int offset = 2 + contador*16;
    memset(&directorio[offset], 0, 14);
    std::memcpy(&directorio[offset], nombre_figura.c_str(), std::min((size_t)14, nombre_figura.size()));
    directorio[offset+14] = static_cast<char>(nuevo_inicio);
    directorio[offset+15] = static_cast<char>(nuevo_final);
    directorio[TAM_BLOQUE - 1] = contador + 1;

    archivo.seekp(0, std::ios::beg);
    archivo.write(directorio.data(), TAM_BLOQUE);

    // Escribir contenido (máximo 255 bytes para no sobrepasar bloque)
    std::string contenido_corto = contenido.substr(0, TAM_BLOQUE-1);
    archivo.seekp(TAM_BLOQUE + (nuevo_inicio - 1)*TAM_BLOQUE + 1, std::ios::beg);
    archivo.write(contenido_corto.c_str(), contenido_corto.size());

    return "Figura agregada correctamente.\n";
}
