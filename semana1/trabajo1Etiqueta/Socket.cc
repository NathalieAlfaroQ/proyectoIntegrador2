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
// man  = manual
Socket::Socket(char t, bool IPv6)
{ // buscar en consola: man socket // aqui me da el code
  if (IPv6 == true)
  {
    this->BuildSocket(t, IPv6); // Call base class constructor
  }
  else
  {
    this->BuildSocket(t, IPv6);
  }
}

/**
 *  Class destructor
 *
 *  @param     int id: socket descriptor
 *
 **/
Socket::~Socket()
{
  this->Close();
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
{ //////////////

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
 * Read method
 *   use "read" Unix system call (man 3 read)
 *
 * @param      void * buffer: buffer to store data read from socket
 * @param      int size: buffer capacity, read will stop if buffer is full
 *
 **/
// Lee datos de un socket y los almacena en un buffer.
size_t Socket::Read(void *buffer, size_t size)
{ ///////////

  // Reads from socket and saves it onto the char
  int st = read(idSocket, buffer, size);

  if (-1 == st)
  {
    throw std::runtime_error("Socket::Read( void *, size_t )");
  }

  return st;
}

/**
 * Write method
 *   use "write" Unix system call (man 3 write)
 *
 * @param      void * buffer: buffer to store data write to socket
 * @param      size_t size: buffer capacity, number of bytes to write
 *
 **/
// Escribe datos desde un buffer a un socket.
size_t Socket::Write(const void *buffer, size_t size)
{
  // Writes from text into the socket

  size_t st = write(idSocket, buffer, size);

  if (-1 == st)
  {
    throw std::runtime_error("Socket::Write( void *, size_t )");
  }

  return st;
}

/**
 * Escribe una cadena de texto en el socket utilizando el método anterior.
 *   use "write" Unix system call (man 3 write)
 *
 * @param      char * text: text to write to socket
 **/
size_t Socket::Write(const char *text)
{
  // strlen(text) Calcula la longitud de la cadena.
  size_t bytes_written = Write(text, strlen(text));

  if (bytes_written == -1)
  {
    throw std::runtime_error("Writing error");
  }
  return bytes_written;
}