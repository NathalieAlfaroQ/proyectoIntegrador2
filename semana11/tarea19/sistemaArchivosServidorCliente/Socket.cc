/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *  ******   Socket class implementation
  *
  * (Fedora version)
  *
 **/
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>         // sockaddr_in
#include <arpa/inet.h>          // ntohs
#include <unistd.h>		// write, read
#include <cstring>
#include <stdexcept>
#include <stdio.h>		// printf

#include "Socket.h"		// Derived class
#define FAIL    -1

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
Socket::Socket( char t, bool IPv6 ){

   this->BuildSocket( t, IPv6 );      // Call base class constructor

}

/**
  * Constructor para socket aceptado (accept)
  * @param int descriptor: el socket ya conectado
  */
Socket::Socket(int descriptor) {
   this->BuildSocket(descriptor);
}


/**
  *  Class destructor
  *
  *  @param     int id: socket descriptor
  *
 **/
Socket::~Socket() {

}


/**
  * MakeConnection method
  *   use "EstablishConnection" in base class
  *
  * @param      char * host: host address in dot notation, example "10.1.166.62"
  * @param      int port: process address, example 80
  *
 **/
int Socket::MakeConnection( const char * hostip, int port ) {

   return this->EstablishConnection( hostip, port );

}


/**
  * MakeConnection method
  *   use "EstablishConnection" in base class
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int Socket::MakeConnection( const char *host, const char *service ) {

   return this->EstablishConnection( host, service );

}


/**
  * Read method
  *   use "read" Unix system call (man 3 read)
  *
  * @param      void * buffer: buffer to store data read from socket
  * @param      int size: buffer capacity, read will stop if buffer is full
  *
 **/
size_t Socket::Read(void * buffer, size_t size) {
   ssize_t st = read(this->idSocket, buffer, size);
   if (st == 0) {
      throw std::runtime_error("Socket::Read(): conexión cerrada por el servidor");
   }
   if (st == -1) {
      throw std::runtime_error("Socket::Read( void *, size_t ) falló");
   }
   return static_cast<size_t>(st);
}


/**
  * Write method
  *   use "write" Unix system call (man 3 write)
  *
  * @param      void * buffer: buffer to store data write to socket
  * @param      size_t size: buffer capacity, number of bytes to write
  *
 **/
size_t Socket::Write(const void * buffer, size_t size) {
   ssize_t st = write(this->idSocket, buffer, size);
   if (st == -1) {
      throw std::runtime_error("Socket::Write( void *, size_t ) falló");
   }
   return static_cast<size_t>(st);
}


/**
  * Write method
  *   use "write" Unix system call (man 3 write)
  *
  * @param      char * text: text to write to socket
  *
 **/
size_t Socket::Write( const char * text ) {

   return this->Write((const void *)text, strlen(text));
}


/**
  * AcceptiConnection method
  *    use base class to accept connections
  *
  *  @returns   a new class instance
  *
  *  Waits for a new connection to service (TCP mode: stream)
  *
 **/
VSocket* Socket::AcceptConnection() {
    if (!IPv6) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(idSocket, (sockaddr*)&client_addr, &client_len);

        if (client_fd < 0) {
            throw std::runtime_error("Socket::AcceptConnection() failed (IPv4)");
        }
        return new Socket(client_fd);
    } else {
        struct sockaddr_in6 client_addr6;
        socklen_t client_len6 = sizeof(client_addr6);
        int client_fd = accept(idSocket, (sockaddr*)&client_addr6, &client_len6);

        if (client_fd < 0) {
            throw std::runtime_error("Socket::AcceptConnection() failed (IPv6)");
        }
        return new Socket(client_fd);
   }
}
