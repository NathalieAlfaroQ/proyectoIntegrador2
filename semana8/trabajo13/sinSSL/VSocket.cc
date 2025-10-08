/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  ****** VSocket base class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>		// getaddrinfo, freeaddrinfo
#include <unistd.h>		// close
/*
#include <cstddef>
#include <cstdio>

//#include <sys/types.h>
*/
#include "VSocket.h"


/**
  *  Class creator (constructor)
  *     use Unix socket system call
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
void VSocket::BuildSocket( char t, bool IPv6 ){

   int domain = IPv6 ? AF_INET6 : AF_INET;
   int typeSocket = (t == 'd') ? SOCK_DGRAM : SOCK_STREAM;
   int protocol = 0;

   idSocket = socket(domain, typeSocket, protocol);
   if (idSocket == -1) {
      throw std::runtime_error("VSocket::BuildSocket, socket() failed");
   }

   this->IPv6 = IPv6;
   this->type = t;
}

/**
  * BuildSocket method
  *    Initialize the current socket instance using an existing descriptor
  *
  * @param     int descriptor: socket file descriptor already created (e.g., from accept)
  *
  *  Assigns the given descriptor to the current socket instance.
  *  Useful when accepting a connection and wrapping the resulting descriptor
  *  into a VSocket-derived object.
  *
  */
 void VSocket::BuildSocket(int descriptor) {
   this->idSocket = descriptor;
   this->type = 's';    // porque siempre estamos en modo stream
   this->IPv6 = false;  // porque estamos usando IPv4 en este caso
}



/**
  * Class destructor
  *
 **/
VSocket::~VSocket() {
   this->Close();
}


/**
  * Close method
  *    use Unix close system call (once opened a socket is managed like a file in Unix)
  *
 **/
void VSocket::Close() {
   if (idSocket != -1) {
      int st = close(idSocket);
      if (st == -1) {
         throw std::runtime_error("VSocket::Close()");
      }
      idSocket = -1;
   }
}


/**
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
int VSocket::EstablishConnection( const char * hostip, int port ) {
   int st;

   if (!this->IPv6) {
      struct sockaddr_in host4;
      memset(&host4, 0, sizeof(host4));
      host4.sin_family = AF_INET;
      st = inet_pton(AF_INET, hostip, &host4.sin_addr);
      if (st <= 0) {
         throw std::runtime_error("VSocket::EstablishConnection, inet_pton() failed");
      }
      host4.sin_port = htons(port);
      st = connect(idSocket, (struct sockaddr *)&host4, sizeof(host4));
      if (st == -1) {
         throw std::runtime_error("VSocket::EstablishConnection, connect() failed");
      }
      return 0;
   }
   throw std::runtime_error("VSocket::EstablishConnection, IPv6 not implemented yet");
}


/**
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int VSocket::EstablishConnection( const char *host, const char *service ) {
   throw std::runtime_error("VSocket::EstablishConnection");
   return -1;
}


/**
  * Bind method
  *    use "bind" Unix system call (man 3 bind) (server mode)
  *
  * @param      int port: bind a unamed socket to a port defined in sockaddr structure
  *
  *  Links the calling process to a service at port
  *
 **/
int VSocket::Bind(int port) {
   this->port = port;

   if (!IPv6) {
      struct sockaddr_in addr;
      memset(&addr, 0, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = INADDR_ANY;
      addr.sin_port = htons(port);

      int opt = 1;
      setsockopt(idSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

      int st = bind(idSocket, (struct sockaddr*)&addr, sizeof(addr));
      if (st == -1) {
         throw std::runtime_error("VSocket::Bind failed (IPv4)");
      }
   } else {
      struct sockaddr_in6 addr6;
      memset(&addr6, 0, sizeof(addr6));
      addr6.sin6_family = AF_INET6;
      addr6.sin6_addr = in6addr_any;
      addr6.sin6_port = htons(port);
      
      int opt = 1;
      setsockopt(idSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

      int st = bind(idSocket, (struct sockaddr*)&addr6, sizeof(addr6));
      if (st == -1) {
          throw std::runtime_error("VSocket::Bind failed (IPv6)");
      }
  }

   return 0;
}


/**
  * MarkPassive method
  *    use "listen" Unix system call (man listen) (server mode)
  *
  * @param      int backlog: defines the maximum length to which the queue of pending connections for this socket may grow
  *
  *  Establish socket queue length
  *
 **/
int VSocket::MarkPassive(int backlog) {
    if (listen(idSocket, backlog) < 0) {
        throw std::runtime_error("VSocket::MarkPassive failed");
    }
    return 0;
}


/**
  * WaitForConnection method
  *    use "accept" Unix system call (man 3 accept) (server mode)
  *
  *
  *  Waits for a peer connections, return a sockfd of the connecting peer
  *
 **/
int VSocket::WaitForConnection( void ) {
   int st = -1;

   throw std::runtime_error( "VSocket::WaitForConnection" );

   return st;

}


/**
  * Shutdown method
  *    use "shutdown" Unix system call (man 3 shutdown) (server mode)
  *
  *
  *  cause all or part of a full-duplex connection on the socket associated with the file descriptor socket to be shut down
  *
 **/
int VSocket::Shutdown( int mode ) {
   if (shutdown(idSocket, mode) < 0) {
      throw std::runtime_error("VSocket::Shutdown failed");
  }
  return 0;
}


// UDP methods 2025

/**
  *  sendTo method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to send data
  *
  *  Send data to another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::sendTo(const void* buffer, size_t len, void* dest) {
   int st;
   if (!IPv6) {
      st = sendto(idSocket, buffer, len, 0, (struct sockaddr*)dest, sizeof(struct sockaddr_in));
   } else {
      st = sendto(idSocket, buffer, len, 0, (struct sockaddr*)dest, sizeof(struct sockaddr_in6));
   }

   if (st == -1) {
      throw std::runtime_error("VSocket::sendTo failed");
   }
   return st;
}


/**
  *  recvFrom method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to receive from data
  *
  *  @return	size_t bytes received
  *
  *  Receive data from another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::recvFrom(void* buffer, size_t len, void* source) {
   socklen_t addr_len = IPv6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in);
   int st = recvfrom(idSocket, buffer, len, 0, (struct sockaddr*)source, &addr_len);

   if (st == -1) {
      throw std::runtime_error("VSocket::recvFrom failed");
   }
   return st;
}

