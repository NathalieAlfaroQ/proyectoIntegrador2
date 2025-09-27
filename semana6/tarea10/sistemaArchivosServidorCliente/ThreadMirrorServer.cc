// ServerPersistent.cpp
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <cstring>

#include "Socket.h"
#include "lector_figura.h" // debe declarar: leer_figura, agregar_figura, listar_figuras

using namespace std;

#define PORT 1234
#define BUFSIZE 2048

void task(VSocket *client)
{
    char a[BUFSIZE];
    // Vamos a leer en un bucle: cada Read = un mensaje del cliente
    while (true) {
        memset(a, 0, BUFSIZE);
        int n = client->Read(a, BUFSIZE);
        if (n <= 0) {
            // cliente cerró o hubo error -> salimos del bucle
            break;
        }

        std::string mensaje(a, n);
        // quitar CR/LF finales
        while (!mensaje.empty() && (mensaje.back() == '\n' || mensaje.back() == '\r')) mensaje.pop_back();

        std::string respuesta;

        if (mensaje.rfind("GET:", 0) == 0) {
            std::string nombre = mensaje.substr(4);
            std::cout << "Servidor: GET '" << nombre << "'\n";
            respuesta = leer_figura(nombre);
        }
        else if (mensaje.rfind("ADD:", 0) == 0) {
            size_t p1 = mensaje.find(":", 4);
            if (p1 == std::string::npos) {
                respuesta = "Formato inválido para ADD.\n";
            } else {
                std::string nombre = mensaje.substr(4, p1 - 4);
                std::string contenido = mensaje.substr(p1 + 1);
                std::cout << "Servidor: ADD nombre='" << nombre << "' tamaño=" << contenido.size() << " bytes\n";
                respuesta = agregar_figura(nombre, contenido);
            }
        }
        else if (mensaje.rfind("LIST", 0) == 0) {
            std::cout << "Servidor: LIST\n";
            auto figuras = listar_figuras();
            respuesta = "Figuras disponibles:\n";
            for (auto &f : figuras) respuesta += "- " + f + "\n";
        }
        else if (mensaje.rfind("QUIT", 0) == 0) {
            respuesta = "BYE\n";
            client->Write(respuesta.c_str());
            break; // cerramos la conexión del servidor con ese cliente
        }
        else {
            respuesta = "Comando no reconocido. Usa GET:Nombre, ADD:Nombre:Contenido, LIST o QUIT\n";
        }

        // Enviamos respuesta y seguimos esperando más mensajes del mismo cliente
        client->Write(respuesta.c_str());
    }

    client->Close();
    // thread termina y se limpia
}

int main(int argc, char **argv)
{
   thread *worker;
   VSocket *s1, *client;

   s1 = new Socket('s', false);
   s1->Bind(PORT);
   s1->MarkPassive(5);

   for (;;) {
      client = s1->AcceptConnection();
      worker = new thread(task, client);
      worker->detach();
   }
}
