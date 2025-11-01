#include <iostream>
#include <cstring>
#include <string>
#include "Socket.h"

#define PORT 5013     // Puerto al que se conectará el cliente
#define BUFSIZE 4096  // Tamaño del búfer para lectura de datos

int main(int argc, char* argv[]) {
    // Verifica si hay al menos un argumento adicional
    if (argc < 2) {
        std::cerr << "[Uso]: " << argv[0] << " <list | add <figura> | <nombre_figura_sin_.txt>>\n";
        return 1;
    }

    std::string comando = argv[1];     // Primer argumento como comando
    std::string host = "10.1.35.5";    // Dirección del servidor (cambiar)
    std::string request;               // Petición HTTP a enviar

    // Construye la solicitud HTTP dependiendo del comando recibido
    if (comando == "list") {
        // Solicitud para listar todas las figuras
        request = "GET /listado HTTP/1.1\r\n"
                  "Host: localhost\r\n"
                  "Connection: close\r\n"
                  "\r\n";
    }
    else if (comando == "add" && argc == 3) {
        // Solicitud para agregar una nueva figura al servidor
        std::string figura = argv[2];
        request = "GET /add?nombre=" + figura + " HTTP/1.1\r\n"
                  "Host: localhost\r\n"
                  "Connection: close\r\n"
                  "\r\n";
    }
    else {
        // Solicitud para obtener una figura específica (usando su nombre)
        request = "GET /figura?nombre=" + comando + " HTTP/1.1\r\n"
                  "Host: localhost\r\n"
                  "Connection: close\r\n"
                  "\r\n";
    }

    try {
        Socket client('s');                         // Crea un socket tipo cliente
        client.MakeConnection(host.c_str(), PORT);  // Conecta al servidor
        client.Write(request.c_str());              // Envía la petición HTTP

        char buffer[BUFSIZE];   // Búfer para almacenar respuesta del servidor
        int bytesRead;

        std::cout << "\n[Respuesta del servidor]:\n\n";

        try {
            // Lee los datos recibidos en bloques hasta que no quede nada
            do {
                memset(buffer, 0, BUFSIZE);              // Limpia el búfer
                bytesRead = client.Read(buffer, BUFSIZE); // Lee datos del socket
                if (bytesRead > 0) {
                    std::cout << std::string(buffer, bytesRead); // Imprime datos
                }
            } while (bytesRead > 0);
        } catch (std::exception& ex) {
            // Maneja errores de lectura (por ejemplo, cierre inesperado del servidor)
            std::string msg = ex.what();
            if (msg.find("conexión cerrada") == std::string::npos) {
                std::cerr << "[ERROR]: " << msg << std::endl;
            }
        }

        client.Close(); // Cierra la conexión del socket

    } catch (std::exception& e) {
        // Manejo de errores generales de conexión o escritura
        std::cerr << "[ERROR]: " << e.what() << std::endl;
    }

    return 0;
}
