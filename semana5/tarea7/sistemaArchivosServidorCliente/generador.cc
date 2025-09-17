/*
Este código es para generar correctamente un archivo binario de las figuras a utilizar
que además calcula la cantidad de bloques que necesita cada figura.
*/

// Bibliotecas
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>

// Tamaño de cada bloque 256 bytes
const int TAM_BLOQUE = 256;
// Para almacenar el contenido en un archivo binario
const std::string ARCHIVO = "almacenamiento.bin";

// Estructura para cada entrada del directorio
struct EntradaDirectorio {
    char nombre[14];   // Nombre de la figura (14 bytes máximo)
    uint8_t inicio;    // Bloque inicial
    uint8_t final;     // Bloque final
};

void guardar_figuras(const std::vector<std::pair<std::string, std::string>>& figuras) {

    std::ofstream archivo(ARCHIVO, std::ios::binary | std::ios::trunc);

    if (!archivo) {
        std::cerr << "No se pudo crear el archivo binario." << std::endl;
        return;
    }

    // Crea directorio vacío que es el bloque 0, inicial
    std::vector<char> directorio(TAM_BLOQUE, 0);

    // Primeros 2 bytes reservados 
    int offset = 2;

    // Bloque donde comienzan a guardarse las figuras
    int bloque_actual = 1;

    // Procesar cada figura
    for (size_t i = 0; i < figuras.size(); ++i) {

        const std::string& nombre = figuras[i].first;
        const std::string& contenido = figuras[i].second;

        // Calcular cuántos bloques ocupa la figura
        int bloques_necesarios = (contenido.size() + TAM_BLOQUE - 1) / TAM_BLOQUE;

        // Preparar entrada de directorio
        EntradaDirectorio entrada{};
        std::memset(entrada.nombre, 0, sizeof(entrada.nombre));
        std::strncpy(entrada.nombre, nombre.c_str(), sizeof(entrada.nombre) - 1);
        entrada.inicio = bloque_actual;
        entrada.final = bloque_actual + bloques_necesarios - 1;

        // Copiar entrada en el directorio
        std::memcpy(&directorio[offset], &entrada, sizeof(entrada));
        // Cada entrada ocupa 16 bytes
        offset += 16; 

        // Escribir contenido de la figura en los bloques correspondientes
        int byte_inicio = TAM_BLOQUE + (entrada.inicio - 1) * TAM_BLOQUE + 1;
        archivo.seekp(byte_inicio, std::ios::beg);
        archivo.write(contenido.c_str(), contenido.size());

        // Siguiente bloque
        bloque_actual += bloques_necesarios; 
    }

    // Escribir el contador de figuras en el último byte del directorio
    directorio[TAM_BLOQUE - 1] = static_cast<char>(figuras.size());

    // Escribir directorio en el archivo
    archivo.seekp(0, std::ios::beg);
    archivo.write(directorio.data(), TAM_BLOQUE);

    archivo.close();
    std::cout << "Archivo binario con " << figuras.size() << " figuras." << std::endl;
}

int main() {

    // Se coloca el nombre de la figura y la figura
    std::vector<std::pair<std::string, std::string>> figuras = {
        {"perro", 
R"(   ___
 __/_  `.  .-"""-.
 \_,` | \-'  /   )`-')
  "") `"`    \  ((`"`
 ___Y  ,    .'7 /
(_,___/...-` (_/_/ sk)"},

        {"torre",
R"(__________________¶¶
_________________¶¶¶¶
_________________¶¶¶¶
__________________¶¶
__________________¶¶
__________________¶¶
__________________¶¶
__________________¶¶
__________________¶¶
_________________¶¶¶¶
_________________¶¶¶¶
_________________¶¶¶¶
_________________¶¶¶¶
_________________¶¶¶¶
_________________¶¶¶¶
_________________¶¶¶¶
_________________¶¶¶¶
_________________¶¶¶¶
_________________¶¶¶¶
_________________¶¶¶¶¶¶
_________________¶¶¶¶¶¶
_________________¶¶¶¶¶¶
_________________¶¶¶¶¶¶
_______________¶¶¶¶¶¶¶¶
_______________¶¶¶¶¶¶¶¶
_______________¶¶¶¶¶¶¶¶
_______________¶¶¶¶¶¶¶¶
______________¶¶¶¶¶¶¶¶¶¶
______________¶¶¶¶¶¶¶¶¶¶
____________¶¶¶¶¶¶¶¶¶¶¶¶¶¶
____________¶¶¶¶¶¶¶¶¶¶¶¶¶¶
_____________¶¶¶¶¶¶¶¶¶¶¶¶
_____________¶¶¶¶¶¶¶¶¶¶¶¶
____________¶¶¶¶¶¶¶¶¶¶¶¶¶¶
____________¶¶¶¶¶_____¶¶¶¶
___________¶¶¶¶¶______¶¶¶¶¶
___________¶¶¶¶¶_______¶¶¶¶
__________¶¶¶¶¶¶_______¶¶¶¶¶
_________¶¶¶¶¶¶________¶¶¶¶¶
______¶¶¶¶¶¶¶¶¶_¶_¶_¶_¶_¶¶¶¶¶¶¶
______¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶
______¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶
______¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶
_____¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶
____¶¶¶¶¶¶¶¶¶¶__________¶¶¶¶¶¶¶¶¶¶
___¶¶¶¶¶¶¶¶________________¶¶¶¶¶¶¶¶
__¶¶¶¶¶¶¶¶__________________¶¶¶¶¶¶¶¶
_¶¶¶¶¶¶¶¶____________________¶¶¶¶¶¶¶¶
¶¶¶¶¶¶¶¶______________________¶¶¶¶¶¶¶¶)"},

        {"hanukka",
R"( __  __  __  __  \__/  __  __  __  __
\__/\__/\__/\__/  )(  \__/\__/\__/\__/
 )(  )(  )(  )(   )(   )(  )(  )(  )(
 )(  )(  )(   \\__)(__//   )(  )(  )(
 )(   \\  \\   `--)(--'   //  //   )(
  \\   \\  \\_____)(_____//  //   //
   \\   \\  `-----)(-----'  //   //
    \\   \\_______)(_______//   //
     \\   `-------)(-------'   //
      \\__________)(__________//
       `----------)(.---------'
                  )(
                 _/\_
                 >()< 
                  \/
                  )(
                 (())
            ___.-"^^"-.___
           '==============`    hjw)"}
    };

    guardar_figuras(figuras);
    
    return 0;
}  