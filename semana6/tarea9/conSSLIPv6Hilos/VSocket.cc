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
#include <netdb.h>			// getaddrinfo, freeaddrinfo
#include <unistd.h>			// close
#include <errno.h> // For errno and strerror
#include <net/if.h> // For if_nametoindex
#include <netinet/in.h> // For IPV6_V6ONLY
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
   int st = -1;
   int protocol = AF_INET;
   if (IPv6) {
      protocol = AF_INET6;
   }
   if (t == 's') {
      st = socket(protocol, SOCK_STREAM, IPPROTO_TCP);
   } else if (t == 'd') {
      st = socket(protocol, SOCK_DGRAM, IPPROTO_UDP);
   } else {
      throw std::runtime_error( "VSocket::BuildSocket: unknown socket type: <" + std::string(1,t) + ">" );
   }
   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::BuildSocket: error creating socket" );
   } else {
      this->idSocket = st;
      this->type = t;
      this->IPv6 = IPv6;
      printf("[VSocket] Created socket with family %s\n", IPv6 ? "AF_INET6" : "AF_INET");
      if (IPv6) {
         int opt = 1;
         if (setsockopt(this->idSocket, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) < 0) {
            perror("[VSocket] setsockopt IPV6_V6ONLY failed");
         } else {
            printf("[VSocket] IPV6_V6ONLY set to 1\n");
         }
      }
   }

}


void VSocket::CreateSocket( int fd , VSocket* & parent) {
      this->idSocket = fd;
      this->type = parent->type;
      this->IPv6 = parent->IPv6;
      this->port = parent->port;
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
void VSocket::Close(){
   int st = -1;
   int fd = this->idSocket;
   if (0 <= fd) {
      st = close(this->idSocket);
   }

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Close()" );
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

   int st = -1;

   // For IPv6
   if (this->IPv6) {
      struct sockaddr_in6 address;
      memset(&address, 0, sizeof(struct sockaddr_in6));
      address.sin6_family = AF_INET6;
      address.sin6_port = port;

      // Handle scope id (interface) if present
      std::string ipstr(hostip);
      size_t percent = ipstr.find('%');
      std::string addr_part = ipstr;
      if (percent != std::string::npos) {
         addr_part = ipstr.substr(0, percent);
         std::string iface = ipstr.substr(percent + 1);
         address.sin6_scope_id = if_nametoindex(iface.c_str());
         if (address.sin6_scope_id == 0) {
            throw std::runtime_error("VSocket::EstablishConnection: invalid interface name for scope id: " + iface);
         }
      }
      int error = inet_pton(AF_INET6, addr_part.c_str(), &address.sin6_addr);
      if ( error != 1) {
         throw std::runtime_error( std::string("VSocket::EstablishConnection: error in inet_pton. Errno: ") + std::to_string(errno) + " - " + strerror(errno) + ", input: " + addr_part );
      }
      st = connect(this->idSocket, (sockaddr*)&address, sizeof(address));
   // For IPv4
   } else {
      struct sockaddr_in address;
      memset(&address, 0, sizeof(struct sockaddr_in));
      address.sin_family = AF_INET;
      // Nota: usar htons (host to network short) para el puerto.
      address.sin_port = htons(port);
      // Store the host address in 4 bytes
      // Using inet's presentation to network function.
      // Converts a text (dot notation) to binary
      int error = inet_pton(AF_INET, hostip, &address.sin_addr.s_addr);
      if ( error != 1) {
         throw std::runtime_error( "VSocket::EstablishConnection: error in inet_pton IPv4" );
      }

      // Conectar el socket con el servidor
      st = connect(this->idSocket, (sockaddr*)&address, sizeof(address));
   }

   if ( -1 == st ) {
      std::string addr_info = this->IPv6 ? hostip : hostip;
      throw std::runtime_error( std::string("VSocket::EstablishConnection: connect failed. Errno: ") + std::to_string(errno) + " - " + strerror(errno) + ", address: " + addr_info );
   } else {
      // In case of success, save the port.
      this->port = port;
   }

   return st;

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
   int st = -1;

   // Get the port associated to the service
   // char* protocol_name;
   int protocol_number;
   int socktype;
   if (this->type == 's') {
      // protocol_name = "tcp";
      protocol_number = IPPROTO_TCP;
      socktype = SOCK_STREAM;

   } else if (this->type == 'd') {
      // protocol_name = "udp";
      protocol_number = IPPROTO_UDP;
      socktype = SOCK_DGRAM;
   }
   struct addrinfo hints, *result, *indPtr;

   // Definir los hints
   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_protocol = protocol_number;
   hints.ai_family = (this->IPv6? AF_INET6: AF_INET);
   hints.ai_socktype = socktype;
   hints.ai_flags = 0;
   st = getaddrinfo(host, service, &hints, &result);
   if (st != 0) {
      throw std::runtime_error( "VSocket::EstablishConnection: getaddrinfo");
   }

   for (indPtr = result; indPtr != nullptr; indPtr = indPtr->ai_next) {
      st = connect(this->idSocket, indPtr->ai_addr, indPtr->ai_addrlen);
      // Successful connection
      if (st == 0) {
         break;
      // Try again with another socket
      } else {
         close(this->idSocket);
         this->idSocket = socket(indPtr->ai_family, indPtr->ai_socktype, indPtr->ai_protocol);
         if (this->idSocket < 0 ) {
            throw std::runtime_error( "VSocket::EstablishConnection: failed socket" );
         }
      }
   }

   freeaddrinfo(result);

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::EstablishConnection: no connection" );
   }

   return st;

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
int VSocket::Bind( int port ) {
   int st = -1;

   if (this->IPv6){
      // IPv6 Bind
      struct sockaddr_in6 host6;
      memset(&host6, '\0', sizeof(host6));
      host6.sin6_family = AF_INET6;
      host6.sin6_port = htons(port);
      host6.sin6_addr = in6addr_any;

      st = bind(this->idSocket, (struct sockaddr*) &host6, sizeof(host6));
   } else {
      // IPv4 Bind
      struct sockaddr_in host4;
      host4.sin_family = AF_INET;
      host4.sin_port = htons(port);
      host4.sin_addr.s_addr = htonl(INADDR_ANY);
      memset(host4.sin_zero, '\0', sizeof(host4.sin_zero));

      st = bind(this->idSocket, (struct sockaddr*) &host4, sizeof(host4));
   }

   if (st != 0) {
      throw std::runtime_error( std::string("VSocket::Bind: failed to bind. Errno: ") + std::to_string(errno) + " - " + strerror(errno) );
   }


   return st;

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
int VSocket::MarkPassive( int backlog ) {
   int st = -1;

   st = listen(this->idSocket, backlog);

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::MarkPassive" );
   }
   return st;

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
   const int sockfd = accept(this->idSocket, NULL, NULL);

   if (sockfd <= 0) {
      throw std::runtime_error( "VSocket::WaitForConnection" );
   }

   return sockfd;

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
   int st = shutdown(this->idSocket, mode);
   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Shutdown" );
   }
   return st;

}


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
size_t VSocket::sendTo( const void * buffer, size_t size, void * addr ) {
   ssize_t  st = -1;

   // Set length of the address
   socklen_t addrlen = 0;
   if (this->IPv6) {
       addrlen = sizeof(sockaddr_in6); // IPv6 address size
   } else {
       addrlen = sizeof(sockaddr_in); // IPv4 address size
   }

   //! I assume UDP will send the whole message ...
   st = sendto(this->idSocket, buffer, size, 0, (sockaddr*)addr, addrlen);

   if (st == -1) {
      throw std::runtime_error( "VSocket::sentTo: failed to send message. Errno: "
                  + std::string(strerror(errno)));
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
size_t VSocket::recvFrom( void * buffer, size_t size, void * addr ) {
   ssize_t  st = -1;

   // Set length of the address
   socklen_t addrlen = 0;
   if (this->IPv6) {
       addrlen = sizeof(sockaddr_in6); // IPv6 address size
   } else {
       addrlen = sizeof(sockaddr_in); // IPv4 address size
   }

   //! I assume UDP will receive the whole message ...
   st = recvfrom(this->idSocket, buffer, size, 0, (sockaddr*)addr, &addrlen);

   if (st == -1) {
      throw std::runtime_error( "VSocket::sentTo: failed to receive message. Errno: "
                  + std::string(strerror(errno)));
   }

   return st;

}

