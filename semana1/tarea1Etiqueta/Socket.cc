/**
 * Interfaz para manejar sockets en sistemas Unix, permitiendo conexiones
 * y transferencia de datos a través de la red.
 * 
 * Crear un socket en IPv4 o IPv6.
 * Establecer conexión a un servidor usando IP o dominio.
 * Leer datos desde el socket.
 * Enviar datos al socket.
 **/

#include <sys/socket.h> // sockaddr_in para manejar y crear sockets.
#include <arpa/inet.h>  // ntohs para conversiones de direcciones IP.
#include <unistd.h>     // Para write y read.
#include <cstring>      // Manejo de cadenas de caracteres.
#include <stdexcept>    // Manejo de excepciones.
#include <stdio.h>      // printf.

// Encabezado
#include "Socket.h"

/**
 *  Clase constructor usa Unix socket system call.
 *
 *  @param     char t: Tipo de socket.
 *     's' para stream
 *     'd' para datagram
 *  @param     bool ipv6: Indica si usa IPv6 o no.
 **/
Socket::Socket(char t, bool IPv6)
{
   // Llama a la clase base.
   this->BuildSocket(t, IPv6);
}

// Destructor.
Socket::~Socket() {}

/**
 * Para establecer una conexión al servidor.
 * EstablishConnection() se encarga de la conexión.
 *
 * @param      char * host: Dirección de host en notación de puntos, ejemplo "10.1.166.62".
 * @param      int port: Dirección del proceso, ejemplo 80.
 **/
int Socket::MakeConnection(const char *hostip, int port)
{
   return this->EstablishConnection(hostip, port);
}

/**
 * Para establecer una conexión al servidor.
 * EstablishConnection() se encarga de la conexión.
 *
 * @param      char * host: Dirección de host en notación de puntos, ejemplo "os.ecci.ucr.ac.cr".
 * @param      char * service: Dirección del proceso, ejemplo "http".
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