#include <iostream>
#include <thread>
#include <fstream>
#include <cstring>

#include "Socket.h"
#include "FileSystem.h"

#define PORT 1234        // Puerto donde el servidor escucha conexiones
#define BUFSIZE 256      // Tamaño del buffer de lectura/escritura

// Función que se ejecuta por cada cliente conectado
void task(VSocket* client) {
    FileSystem fs("FileSystem.bin"); // Inicializa el sistema de archivos con el archivo binario
    bool connection = true;          // Controla si la conexión sigue activa
    char buffer[BUFSIZE];            // Buffer para lectura de datos del cliente

    while (connection) {
        memset(buffer, 0, BUFSIZE);     // Limpia el buffer
        client->Read(buffer, BUFSIZE);  // Lee un mensaje del cliente
        std::string request(buffer);    // Convierte el buffer a string

        // Comando para listar todas las figuras disponibles
        if (request == "list") {
            std::cout << "[!] Solicitando listar figuras de File System" << std::endl;
            std::string listado = fs.Listar_Figuras();  // Llama al método para obtener lista
            client->Write(listado.c_str());             // Envía la lista al cliente
        }
        // Comando para obtener una figura específica
        else if (request.rfind("get%", 0) == 0) {
            std::cout << "[!] Solicitando leer figura" << std::endl;
            std::string name = request.substr(4);       // Extrae el nombre de la figura
            std::string figura = fs.Leer_Figura(name);  // Intenta leer la figura

            // Si se detecta que no se encontró la figura (por el mensaje del sistema de archivos)
            if (figura.find("[INFO] Figura") != std::string::npos) {
                client->Write("-1");  // Señal de figura no encontrada
            } else {
                // Envía la figura por partes (en caso de que sea muy grande)
                for (size_t i = 0; i < figura.size(); i += BUFSIZE) {
                    std::string parte = figura.substr(i, BUFSIZE);
                    client->Write(parte.c_str(), parte.size());
                }
                client->Write("END");  // Indica al cliente que se terminó de enviar
            }
        }
        // Comando para agregar una nueva figura al sistema
        else if (request.rfind("add%", 0) == 0) {
            std::cout << "[!] Solicitando agregar figura" << std::endl;
            std::string ruta = request.substr(4);  // Extrae el nombre/ruta de la figura
            if (fs.Agregar_Figura(ruta)) {
                client->Write("[+] Figura agregada correctamente\n");
            } else {
                client->Write("[!] No se encontró una figura con ese nombre\n");
            }
        }
        // Comando para cerrar la conexión
        else if (request == "disconnect") {
            std::cout << "[!] Solicitando desconexión con servidor" << std::endl;
            client->Write("-2");     // Señal para cerrar la conexión
            connection = false;      // Sale del bucle
        }
        // Comando no reconocido
        else {
            client->Write("[!] Comando no reconocido\n");
        }
    }

    std::cout << "[!] Conexión con el cliente cerrada" << std::endl;
    // Cierra y limpia el socket del cliente
    client->Close();
    delete client;
}

int main(int argc, char** argv) {
    // Crea e inicializa el sistema de archivos si no existe
    FileSystem fs("FileSystem.bin");
    fs.Crear_FileSystem();  // Crea estructura base del filesystem si es necesario

    // Configura el socket del servidor
    Socket* servidor = new Socket('s');  // Crea socket servidor
    servidor->Bind(PORT);                // Asocia el socket al puerto
    servidor->MarkPassive(5);            // Marca el socket como pasivo, con un backlog de 5 conexiones

    // Acepta conexiones indefinidamente
    while (true) {
        VSocket* cliente = servidor->AcceptConnection();  // Espera y acepta una conexión
        std::thread worker(task, cliente);                // Crea un hilo para manejar al cliente
        worker.detach();
    }
    
    return 0;
}
