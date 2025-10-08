#include <stdio.h>
#include <string.h>
#include <iostream>

#include "VSocket.h"
#include "Socket.h"

#define PORT 1234       // Puerto del servidor al que se va a conectar
#define BUFSIZE 256     // Tamaño del buffer de lectura

// Función principal que maneja la conexión y comunicación con el servidor
int run() {
    int st = -1;                // Código de estado de conexión
    Socket* s;                  // Puntero a socket
    char buffer[BUFSIZE];       // Buffer para recibir datos del servidor
    bool connection = true;     // Controla si la conexión sigue activa

    s = new Socket('s');        // Crea un socket cliente

    // Intenta conectar al servidor en la IP dada y el puerto definido
    st = s->MakeConnection("127.0.0.1", PORT);
    if (st < 0) {
        std::cerr << "[!] Error connecting to the server.\n";
        return st;
    }

    // Bucle principal para enviar comandos al servidor
    while (connection) {
        std::string request;
        printf("[!] Comando: ");
        std::getline(std::cin, request);   // Lee el comando del usuario

        // Interpreta el comando del usuario y lo transforma en el formato esperado por el servidor
        if (request == "list") {
            request = "list";  // Lista las figuras en el sistema de archivos
        } else if (request == "disconnect") {
            request = "disconnect";  // Cierra la conexión
        } else if (request.rfind("add ", 0) == 0) {
            // Agrega una figura al sistema de archivos
            std::string path = request.substr(4);    // Extrae el nombre del archivo
            request = "add%" + path;                 // Se usa "%" como separador de comando y parámetro
        } else if (request.rfind("get ", 0) == 0) {
            // Solicita ver una figura
            std::string path = request.substr(4);
            request = "get%" + path;
        }

        // Envía el comando al servidor
        s->Write(request.c_str());

        int bytesRead = 0;
        memset(buffer, 0, BUFSIZE);  // Limpia el buffer antes de leer

        // Lee la respuesta del servidor
        do {
            memset(buffer, 0, BUFSIZE);           // Limpia el buffer
            bytesRead = s->Read(buffer, BUFSIZE); // Lee datos del servidor

            if (bytesRead > 0) {
                std::string responseStr(buffer, bytesRead); // Convierte buffer a string

                // Verifica si el servidor indicó el final de la respuesta
                if (responseStr.find("END") != std::string::npos) {
                    break;
                } 
                // Mensaje de error: figura no encontrada
                else if (responseStr.find("-1") != std::string::npos) {
                    printf("\n[!] Figura no encontrada.\n");
                } 
                // Mensaje de desconexión
                else if (responseStr.find("-2") != std::string::npos) {
                    printf("\n[!] Terminando conexión.\n");
                    connection = false;  // Sale del bucle principal
                } 
                // Muestra la respuesta del servidor
                else {
                    printf("%s", responseStr.c_str());
                }
            }

        // Sigue leyendo mientras el buffer venga lleno (puede haber más datos)
        } while (bytesRead == BUFSIZE);        
        printf("\n");
    }

    // Cierra el socket y libera la memoria
    s->Close();
    delete s;
    return st;
}

// Función main que muestra los comandos disponibles y arranca la aplicación
int main(int argc, char *argv[]) {
    printf("\n------ COMANDOS DISPONIBLES ------\n\n");
    printf("[!] Para ver una figura: escriba 'get <nombre de la figura>'\n");
    printf("[!] Para ver la lista de figuras: escriba 'list'\n");
    printf("[!] Para agregar figura: escriba 'add <nombre de la figura>'\n");
    printf("[!] Para salir: escriba 'disconnect'\n\n");
    run();  // Ejecuta la función principal de cliente
    return 0;
}
