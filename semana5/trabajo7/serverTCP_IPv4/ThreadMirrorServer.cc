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

// Encabezados
#include "Socket.h"

using namespace std;

#define PORT 1234   // Puerto donde escuha el servidor
#define BUFSIZE 250 // Tamaño del mensaje

/**
 * Ejecuta cada hilo creado para atender a un cliente.
 */
void task(VSocket *client)
{
   // Para guardar el mensaje
   char a[BUFSIZE];
   // Leer el mensaje del cliente
   client->Read(a, BUFSIZE);
   // Muestra el mensaje del servidor
   cout << "Servidor recibe: " << a << endl;
   // Le enviamos el mismo mensaje al cliente
   client->Write(a);
   // Cierra la conexion con el cliente
   client->Close();
}

/**
 * Funcion principal del servidor.
 **/
int main(int argc, char **argv)
{
   // Hilo que atiende al cliente
   thread *worker;
   // s1 es el socket del servidor, client es el socket del cliente
   VSocket *s1, *client;

   // Crea el socket del servidor
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