/**
 *
 **/

#include <sys/socket.h> // sockaddr_in
#include <arpa/inet.h>  // ntohs
#include <unistd.h>     // write, read
#include <cstring>
#include <stdexcept>
#include <stdio.h> // printf

#include "Socket.h"

/**
 *  Class constructor
 *     use Unix socket system call
 *
 *  @param     char t: socket type to define
 *     's' for stream
 *     'd' for datagram
 *  @param     bool ipv6: if we need a IPv6 socket
 *
 **/
Socket::Socket(char t, bool IPv6)
{

   this->BuildSocket(t, IPv6); // Call base class constructor
}

/**
 *  Class destructor
 *
 *  @param     int id: socket descriptor
 *
 **/
Socket::~Socket()
{
}

/**
 * MakeConnection method
 *   use "EstablishConnection" in base class
 *
 * @param      char * host: host address in dot notation, example "10.1.166.62"
 * @param      int port: process address, example 80
 *
 **/
int Socket::MakeConnection(const char *hostip, int port)
{
   return this->EstablishConnection(hostip, port);
}

/**
 * MakeConnection method
 *   use "EstablishConnection" in base class
 *
 * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
 * @param      char * service: process address, example "http"
 *
 **/
int Socket::MakeConnection(const char *host, const char *service)
{
   return this->EstablishConnection(host, service);
}

/**
 * Lee datos del socket y los almacena en buffer hasta un máximo de size bytes.
 * Usa "read" Unix system call (man 3 read).
 *
 * @param      void * buffer: Almacena los datos leídos del socket.
 * @param      int size: Capacidad del buffer, si se llena, se detiene.
 **/
size_t Socket::Read(void *buffer, size_t size)
{
   // Retorna el número de bytes leídos.
   int st = read(idSocket, buffer, size);

   // Si da error, lanza una excepción.
   if (-1 == st)
   {
      throw std::runtime_error("Socket::Read( void *, size_t )");
   }

   return st;
}

/**
 * Escribe size bytes desde buffer en el socket.
 * Usa "write" Unix system call (man 3 write).
 *
 * @param      void * buffer: Almacena los datos a escribir del socket.
 * @param      size_t size: Capacidad del buffer, número de bytes a escribir.
 **/
size_t Socket::Write(const void *buffer, size_t size)
{
   // Escribe.
   size_t st = write(idSocket, buffer, size);

   // Si la escritura falla, lanza una excepción.
   if (-1 == st)
   {
      throw std::runtime_error("Socket::Write( void *, size_t )");
   }

   return st;
}

/**
 * Calcula la longitud del texto.
 * Usa "write" Unix system call (man 3 write).
 *
 * @param      char * text: Texto a escribir al socket.
 **/
size_t Socket::Write(const char *text)
{
   // Llama a write y busca el largo de la cadena.
   // strlen(text) Calcula la longitud de la cadena.
   size_t bytes_written = Write(text, strlen(text));

   // Lanzar excepción si hay error.
   if (bytes_written == -1)
   {
      throw std::runtime_error("Writing error");
   }

   return bytes_written;
}