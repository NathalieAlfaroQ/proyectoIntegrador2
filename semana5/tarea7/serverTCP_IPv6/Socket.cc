/*
 * Implementación de la clase Socket.
 *
 * Esta clase maneja operaciones de creación, conexión, lectura y escritura
 * de sockets de manera más sencilla.
 */

// Bibliotecas
#include <sys/socket.h> // sockaddr_in
#include <arpa/inet.h>  // ntohs
#include <unistd.h>     // write, read
#include <cstring>
#include <stdexcept>
#include <stdio.h> // printf

// Encabezados
#include "Socket.h"

using namespace std;

/**
 * Constructor de la clase Socket.
 * Crea un socket según el tipo ('s' para stream TCP, 'd' para datagrama UDP)
 * y el protocolo (IPv4 o IPv6).
 */
Socket::Socket(char t, bool IPv6)
{
   // Llama la función de construcción de la clase base
   this->BuildSocket(t, IPv6);
}

Socket::Socket( int id ) {

  this->BuildSocket( id );

}

/**
 * Destructor de la clase Socket.
 * Cierra el socket si está abierto.
 */
Socket::~Socket()
{
   Close();
}

/**
 * Método para hacer conexión a un servidor, usando IP y puerto.
 */
int Socket::MakeConnection(const char *hostip, int port)
{
   return this->EstablishConnection(hostip, port);
}

/**
 * Método para hacer conexión a un servidor, usando nombre DNS y servicio.
 */
int Socket::MakeConnection(const char *host, const char *service)
{
   return this->EstablishConnection(host, service);
}

/**
 * Método para leer datos del socket.
 */
size_t Socket::Read(void *buffer, size_t size)
{
   int st = -1;
   st = read(idSocket, buffer, size);

   if (-1 == st)
   {
      throw runtime_error("Socket::Read");
   }

   return st;
}

/**
 * Método para escribir datos en el socket (especificando el tamaño).
 */
size_t Socket::Write(const void *buffer, size_t size)
{
   int st = -1;
   st = write(idSocket, buffer, size);

   if (-1 == st)
   {
      throw runtime_error("Socket::Write");
   }

   return st;
}

/**
 * Método para escribir una cadena de texto en el socket.
 */
size_t Socket::Write(const char *text)
{
   int st = -1;
   st = Write(text, strlen(text));

   if (-1 == st)
   {
      throw runtime_error("Socket::Write-text");
   }

   return st;
}

/**
 * Método para aceptar una conexión entrante en modo servidor.
 * Devuelve una nueva instancia de socket para comunicarse con el cliente.
 */
   VSocket *Socket::AcceptConnection()
{
   int id;
   VSocket *peer;
   id = this->WaitForConnection();
   peer = new Socket(id);
   return peer;
}