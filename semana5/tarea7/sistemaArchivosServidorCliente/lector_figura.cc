/*
Este programa es un sistema de archivos sencillo que lee un archivo binario
donde se encuentran las figuras.

Recordar que tenemos bloques de 256 bytes y en el bloque inicial está el directorio
que contiene la lista de figuras existentes, sus nombres y sus bloques ocupados.

Este programa busca una figura en los bloques en que se encuentra y la devuelve.
*/

// Bibliotecas
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring> 
#include <cstdint>

// Tamaño de cada bloque, 256 bytes
const int TAM_BLOQUE = 256;
// Para almacenar el archivo de tipo binario
const std::string ARCHIVO = "almacenamiento.bin";

std::string leer_figura(const std::string &nombre_figura)
{
    // Para abrir el archivo binario
    std::ifstream archivo(ARCHIVO, std::ios::binary);

    // Si no se puede abrir, se notifica
    if (!archivo)
    {
        return "No se pudo abrir el archivo binario.\n";
    }

    // Lee los primeros 256 bytes del primer bloque que es el directorio
    std::vector<char> directorio(TAM_BLOQUE);
    archivo.read(directorio.data(), TAM_BLOQUE);
    // Tiene el último byte del bloque, contador de figuras
    uint8_t contador = static_cast<uint8_t>(directorio[TAM_BLOQUE - 1]);

    // Recorre el directorio
    for (int i = 0; i < contador; ++i)
    {
        // Cada entrada de figura ocupa 16 bytes en el directorio
        // El 2 es para reservar los 2 primeros bytes
        int offset = 2 + i * 16;

        // Copia los primeros 14 bytes, los pasa a string y elimina los vacíos
        char nombre_raw[15] = {0};
        std::memcpy(nombre_raw, &directorio[offset], 14);
        std::string nombre(nombre_raw);
        nombre.erase(nombre.find_last_not_of("\0 ") + 1);

        // Los últimos 2 bytes de la entrada indican el bloque de inicio y final
        uint8_t inicio = static_cast<uint8_t>(directorio[offset + 14]);
        uint8_t final = static_cast<uint8_t>(directorio[offset + 15]);

        // Se centra en una figura en específico
        if (nombre == nombre_figura)
        {
            // Calcula la posición donde inicia y termina
            int byte_inicio = TAM_BLOQUE + (inicio - 1) * TAM_BLOQUE + 1;
            int byte_final = TAM_BLOQUE + final * TAM_BLOQUE;
            int longitud = byte_final - byte_inicio;

            // Mueve el puntero del archivo al inicio de la figura y lee sus caracteres
            archivo.seekg(byte_inicio, std::ios::beg);
            std::vector<char> figura_bytes(longitud);
            archivo.read(figura_bytes.data(), longitud);

            // De bytes a string
            std::string figura(figura_bytes.data(), figura_bytes.size());
            figura.erase(figura.find_last_not_of('\0') + 1);

            return figura;
        }
    }
    
    // Notifica si se solicita una figura que no existe
    return "Figura '" + nombre_figura + "' no encontrada.\n";
}