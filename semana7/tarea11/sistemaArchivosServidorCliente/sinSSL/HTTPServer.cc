#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>
#include <cstring>

#include "Socket.h"
#include "FileSystem.h"

#define PORT 5013
#define BUFSIZE 1024

// Función que maneja cada conexión de cliente de manera concurrente
void task(VSocket* client) {
    FileSystem fs("FileSystem.bin");     // Instancia para manejar figuras en el sistema de archivos
    char buffer[BUFSIZE];
    memset(buffer, 0, BUFSIZE);          // Inicializa el buffer con ceros

    try {
        client->Read(buffer, BUFSIZE);   // Lee la solicitud del cliente
        std::string request(buffer);

        std::cout << "\n========== NUEVA CONEXIÓN ==========\n";
        std::cout << "[RAW REQUEST]:\n" << request << "\n";

        // Extrae el método HTTP, el path solicitado y el protocolo
        std::istringstream reqStream(request);
        std::string method, fullPath, protocol;
        reqStream >> method >> fullPath >> protocol;

        // Solo se permite el método GET
        if (method != "GET") {
            client->Write("HTTP/1.1 405 Method Not Allowed\r\n\r\n");
            client->Close();
            delete client;
            return;
        }

        // Extrae el path y los parámetros de consulta (query string)
        size_t qmark = fullPath.find('?');
        std::string path = fullPath.substr(0, qmark);
        std::string query = (qmark != std::string::npos) ? fullPath.substr(qmark + 1) : "";

        std::cout << "[PATH]: " << path << "\n";
        std::cout << "[QUERY]: " << query << "\n";

        // Manejo de petición para obtener una figura específica
        if (path == "/figura") {
            size_t pos = query.find("nombre=");
            if (pos != std::string::npos) {
                std::string nombre = query.substr(pos + 7);

                // Agregar .txt si no tiene
                if (nombre.find(".txt") == std::string::npos) {
                    nombre += ".txt";
                }

                std::string figura = fs.Leer_Figura(nombre);

                // Si no se encontró la figura, responder 404
                if (figura.find("[INFO] Figura") != std::string::npos) {
                    std::string response = "HTTP/1.1 404 Not Found\r\n\r\nFigura no encontrada\n";
                    client->Write(response.c_str());
                } else {
                    // Genera respuesta HTML con la figura dentro de <pre>
                    std::string cuerpo = "<html><head><meta charset=\"utf-8\"></head><body><pre>\n" + figura + "\n</pre></body></html>\n";
                    std::stringstream ss;
                    ss << "HTTP/1.1 200 OK\r\n"
                       << "Content-Type: text/html; charset=utf-8\r\n"
                       << "Content-Length: " << cuerpo.size() << "\r\n"
                       << "\r\n"
                       << cuerpo;
                    client->Write(ss.str().c_str());
                }
            } else {
                client->Write("HTTP/1.1 400 Bad Request\r\n\r\nFalta parámetro 'nombre'\n");
            }
        }

        // Manejo de solicitud de listado de figuras
        else if (path == "/listado") {
            std::string lista = fs.Listar_Figuras();
            std::string cuerpo = "<html><head><meta charset=\"utf-8\"></head><body><pre>\n" + lista + "\n</pre></body></html>\n";
            std::stringstream ss;
            ss << "HTTP/1.1 200 OK\r\n"
               << "Content-Type: text/html; charset=utf-8\r\n"
               << "Content-Length: " << cuerpo.size() << "\r\n"
               << "\r\n"
               << cuerpo;
            client->Write(ss.str().c_str());
        }

        // Manejo de solicitud para agregar una figura al sistema
        else if (path == "/add") {
            size_t pos = query.find("nombre=");
            if (pos != std::string::npos) {
                std::string nombre = query.substr(pos + 7);

                // Agregar .txt si no tiene
                if (nombre.find(".txt") == std::string::npos) {
                    nombre += ".txt";
                }

                // Intenta agregar la figura desde carpeta Figuras/
                if (fs.Agregar_Figura(nombre)) {
                    std::string response = "HTTP/1.1 200 OK\r\n\r\nFigura agregada correctamente.\n";
                    client->Write(response.c_str());
                } else {
                    std::string response = "HTTP/1.1 404 Not Found\r\n\r\nNo se encontró el archivo Figuras/" + nombre + "\n";
                    client->Write(response.c_str());
                }
            } else {
                client->Write("HTTP/1.1 400 Bad Request\r\n\r\nFalta parámetro 'nombre'\n");
            }
        }

        // Ruta desconocida
        else {
            std::cout << "[WARN] Ruta no reconocida: " << path << "\n";
            client->Write("HTTP/1.1 404 Not Found\r\n\r\nRuta no reconocida\n");
        }

    } catch (std::exception& ex) {
        std::cerr << "[ERROR] " << ex.what() << "\n";
        client->Write("HTTP/1.1 500 Internal Server Error\r\n\r\nError interno\n");
    }

    // Cierra conexión y libera memoria del cliente
    client->Close();
    delete client;
}

// Función principal del servidor
int main(int argc, char** argv) {
    FileSystem fs("FileSystem.bin");
    fs.Crear_FileSystem();   // Crea el sistema de archivos inicial si no existe

    Socket* servidor = new Socket('s');
    servidor->Bind(PORT);        // Asocia el socket al puerto especificado
    servidor->MarkPassive(5);    // Escucha conexiones entrantes (máx. 5 en espera)

    // Bucle infinito para aceptar conexiones
    while (true) {
        VSocket* cliente = servidor->AcceptConnection(); // Acepta cliente
        std::thread worker(task, cliente);               // Atiende al cliente en un hilo separado
        worker.detach();                                 // El hilo se ejecuta de forma independiente
    }

    return 0;
}
