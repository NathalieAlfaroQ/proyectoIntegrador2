/*
 * Ejemplo de socket cliente/servidor usando hilos (threads).
 *
 * Crea un servidor que escucha por conexiones de clientes
 * en un puerto. Cada vez que un cliente se conecta, se crea
 * un nuevo hilo que se encarga de hablar
 * con ese cliente. El servidor recibe un mensaje y se lo devuelve.
 */

// Bibliotecas
#include <iostream>
#include <thread>
#include <string>

// Encabezados
#include "Socket.h"
#include "lector_figura.h"

using namespace std;

#define PORT 1234    // Puerto donde escuha el servidor
#define BUFSIZE 2048 // Tamaño del mensaje

std::string agregar_figura(const std::string &nombre_figura, const std::string &contenido);

/**
 * Ejecuta cada hilo creado para atender a un cliente.
 */
// Reemplaza esta función en tu servidor (usa las mismas includes y defines que ya tienes)
void task(VSocket *client)
{
    char a[BUFSIZE] = {0};

    // Guardamos cuántos bytes recibió el Read (más robusto)
    int n = client->Read(a, BUFSIZE);
    if (n <= 0) {
        // nada recibido o error: cerramos y salimos
        client->Close();
        return;
    }

    // Construimos el string con la longitud real leída
    std::string mensaje(a, n);

    // Respuesta que enviaremos al cliente
    std::string respuesta;

    // No imprimir aquí el mensaje completo (eso hacía que el server mostrara el dibujo)
    if (mensaje.rfind("GET:", 0) == 0) {
        // Comando GET: extraemos nombre y atendemos
        std::string nombre = mensaje.substr(4);
        // limpiamos posibles CR/LF finales
        while (!nombre.empty() && (nombre.back() == '\n' || nombre.back() == '\r')) nombre.pop_back();

        std::cout << "Servidor: GET '" << nombre << "'\n";
        respuesta = leer_figura(nombre); // tu función existente
    }
    else if (mensaje.rfind("ADD:", 0) == 0) {
        // Comando ADD: ADD:Nombre:Contenido... (Contenido puede tener saltos de línea)
        size_t p1 = mensaje.find(":", 4);
        if (p1 == std::string::npos) {
            respuesta = "Formato inválido para ADD.\n";
        } else {
            std::string nombre = mensaje.substr(4, p1 - 4);
            std::string contenido = mensaje.substr(p1 + 1); // todo lo que viene después

            // Evitamos imprimir el 'contenido' completo en el servidor.
            // Solo mostramos info segura (nombre y tamaño):
            std::cout << "Servidor: ADD nombre='" << nombre << "' tamaño=" << contenido.size() << " bytes\n";

            // Llamamos a la función que guarda la figura
            respuesta = agregar_figura(nombre, contenido);

            // ----- OPCIONAL: si quieres que el cliente reciba también el dibujo -----
            // Uncomment la siguiente línea para que la respuesta incluya el contenido
            // respuesta += "\n" + contenido;
            // -----------------------------------------------------------------------
        }
    }
    else {
        respuesta = "Comando no reconocido. Usa GET:Nombre o ADD:Nombre:Contenido\n";
    }

    // Enviamos la respuesta y cerramos el socket del cliente
    client->Write(respuesta.c_str());
    client->Close();
}

/**
 * Función principal del servidor.
 **/
int main(int argc, char **argv)
{
   // Hilo que atiende al cliente
   thread *worker;
   // s1 es el socket del servidor, client es el socket del cliente 
   VSocket *s1, *client;

   // Crea el socket del servidor en IPv4
   s1 = new Socket('s', false);
   // Se vincula al puerto
   s1->Bind(PORT);
   // Modo escucha con capacidad 5 en cola
   s1->MarkPassive(5);

   for (;;)
   {
      // Esperamos una conexión de cliente
      client = s1->AcceptConnection();
      // Creamos un nuevo hilo para atenderlo
      worker = new thread(task, client);
      // Para que el hilo trabaje por su cuenta y el sistema lo limpie después
      worker->detach();
   }
}