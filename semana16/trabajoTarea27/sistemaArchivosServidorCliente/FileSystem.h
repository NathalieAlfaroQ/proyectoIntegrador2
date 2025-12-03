#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>
#include <set>
#include <cstdint>
#include <fstream>

/**
 * Clase FileSystem
 * Esta clase implementa un sistema de archivos binario orientado al almacenamiento de figuras ASCII.
 * Las figuras se almacenan en bloques de tamaño fijo y se organizan en directorios.
 */
class FileSystem {
public:
    // Constantes del sistema
    static const int TAM_BLOQUE = 256;                // Tamaño de cada bloque (en bytes)
    static const int ENTRADAS_POR_DIRECTORIO = 12;    // Cantidad máxima de figuras por bloque de directorio
    static const int TAM_ENTRADA = 14 + 4 + 2;         // Tamaño de una entrada: nombre(14) + inicio(4) + bloques(2)

    /**
     * Estructura que representa una entrada de directorio
     * Contiene el nombre de la figura, el bloque de inicio y la cantidad de bloques usados.
     */
    struct EntradaDirectorio {
        char nombre[14];     // Nombre de la figura (máximo 13 caracteres + nulo)
        uint32_t inicio;     // Bloque donde empieza la figura
        uint16_t bloques;    // Número de bloques ocupados por la figura
    };

    /**
     * Constructor que recibe la ruta del archivo binario que simula el disco
     */
    FileSystem(const std::string& archivo);

    /**
     * Crea e inicializa el sistema de archivos binario
     * Bloque 0 se reserva para registrar los directorios
     * Bloque 1 se asigna como primer directorio
     */
    void Crear_FileSystem();

    /**
     * Agrega una figura desde un archivo de texto al sistema de archivos y retorna true si salió bien o false
     * si no
     */
    bool Agregar_Figura(const std::string& rutaFigura);

    /**
     * Busca una figura por su nombre y la retorna como string si existe
     */
    std::string Leer_Figura(const std::string& ruta);

    /**
     * Escanea el sistema de archivos y retorna los bloques actualmente ocupados
     */
    std::set<uint32_t> Obtener_Bloques_Ocupados();

    /**
     * Hace una lista de las figuras que están disponibles
     */
    std::string Listar_Figuras();

private:
    std::string archivoBin;  // Ruta del archivo binario que representa el disco

    /**
     * Crea un nuevo bloque de directorio y lo registra en el bloque 0
     * @param archivo archivo binario abierto
     * @param bloque0 contenido del bloque 0
     * @return el número de bloque del nuevo directorio
     */
    uint32_t Crear_Directorio(std::fstream& archivo, std::vector<char>& bloque0);

    /**
     * Busca un directorio con espacio libre, o crea uno nuevo si todos están llenos
     * @param archivo archivo binario abierto
     * @param bloque0 contenido del bloque 0
     * @return bloque del directorio encontrado o creado
     */
    uint32_t Encontrar_Crear_Directorio(std::fstream& archivo, std::vector<char>& bloque0);

    /**
     * Lee el contenido de un archivo de texto y lo devuelve como string
     * @param nombre_archivo ruta del archivo a leer
     * @return contenido del archivo en formato string
     */
    std::string Leer_Archivo(const std::string& nombre_archivo);
};

#endif
