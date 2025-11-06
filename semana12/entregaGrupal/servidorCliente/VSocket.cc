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
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>			// getaddrinfo, freeaddrinfo
#include <unistd.h>			// close
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

  this -> IPv6 = IPv6;
  type = t == 's'? SOCK_STREAM : SOCK_DGRAM;
  idSocket = socket(IPv6? AF_INET6 : AF_INET, type, 0);

  if ( -1 == idSocket ) {
    throw std::runtime_error( "VSocket::BuildSocket, no se ha podido asignar un idsocket" );
  }

}

void VSocket::BuildSocket( int id ){
  int st = -1;
  idSocket = id;

  struct sockaddr_storage addr;
  socklen_t addrlen = sizeof(addr);

  st = getsockname(id, (struct sockaddr*) &addr, &addrlen);

  if ( -1 == st ) {
    throw std::runtime_error( "VSocket::BuildSocket, no hay socket asignado a tal id" );
  }

  IPv6 = addr.ss_family == AF_INET6;
  addrlen = sizeof(type);
  st = getsockopt(idSocket, SOL_SOCKET, SO_TYPE, &type, &addrlen);

  
  if ( -1 == st ) {
    throw std::runtime_error( "VSocket::BuildSocket, no se ha podido determinar un tipo" );
  }
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
   int st = close(idSocket);

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

    if (IPv6){
    struct sockaddr_in6  host6;
    memset( (char *) &host6, 0, sizeof( host6 ) );
    host6.sin6_family = AF_INET6;

    st = inet_pton( AF_INET6, hostip, &host6.sin6_addr );
    if ( st <= 0 ) {
      if (st == 0) {
        throw std::runtime_error( "VSocket::DoConnect, inet_pton (invalid IPv6 address)" );
      } else {
        throw std::runtime_error( "VSocket::DoConnect, inet_pton" );
      }
    }

    host6.sin6_port = htons( port );
    st = connect( idSocket, (sockaddr *) &host6, sizeof( host6 ) );
    if ( -1 == st ) {
      perror("VSocket::DoConnect - connect IPv6");
      throw std::runtime_error( "VSocket::DoConnect, 6connect" );
    }

  } else {
    struct sockaddr_in  host4;
    memset( (char *) &host4, 0, sizeof( host4 ) );
    host4.sin_family = AF_INET;

    st = inet_pton( AF_INET, hostip, &host4.sin_addr );
    if ( st <= 0 ) {
      if (st == 0) {
        throw std::runtime_error( "VSocket::DoConnect, inet_pton (invalid IPv4 address)" );
      } else {
        throw std::runtime_error( "VSocket::DoConnect, inet_pton" );
      }
    }

    host4.sin_port = htons( port );
    st = connect( idSocket, (sockaddr *) &host4, sizeof( host4 ) );
    if ( -1 == st ) {
      throw std::runtime_error( "VSocket::DoConnect, 4connect" );
    }
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
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = type;
  hints.ai_protocol = 0;          /* Any protocol */

  st = getaddrinfo(host, service, &hints, &result);
  if (st != 0) {
    throw std::runtime_error("getaddrinfo failed");
  }

  switch(st) {
    case EAI_NONAME:
      printf("EAI_NONAME");
      break;
    case EAI_SERVICE:
      printf("EAI_SERVICE");
      break;
    case  EAI_FAMILY:
      printf(" EAI_FAMILY");
      break;
  }

  if (st != 0) {
    throw std::runtime_error( "VSocket::EstablishConnectionDNS" );
  }

  int last_errno = 0;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if (IPv6){
      if (rp->ai_family != AF_INET6) continue;
    } else {
      if (rp->ai_family != AF_INET) continue;
    }

    if (type != rp->ai_socktype) {
      continue;
    }

    
    st = connect(idSocket, rp->ai_addr, rp->ai_addrlen);
    if (st == 0) {
      break; // conectado correctamente
    } else {
      last_errno = errno;
    }
  }

  freeaddrinfo(result);

  if (st != 0) {
    fprintf(stderr, "VSocket::EstablishConnection: all connect attempts failed, errno=%d (%s)\n",
        last_errno, strerror(last_errno));
    throw std::runtime_error( "VSocket::EstablishConnectionDirection" );
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

   int opt = 1;
   if (setsockopt(this->idSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    throw std::runtime_error("setsockopt SO_REUSEADDR failed");
   }

   if(IPv6) {
    struct sockaddr_in6  host6;
    memset( (char *) &host6, 0, sizeof( host6 ) );
    host6.sin6_family = AF_INET6;
    host6.sin6_addr = in6addr_any;
    host6.sin6_port = htons( port );

    st = bind(idSocket, (struct sockaddr*)&host6, sizeof(host6));

    if (st != 0) {
      throw std::runtime_error( "VSocket::6Bind" );
    }
    
   } else {

    struct sockaddr_in  host4;
    memset( (char *) &host4, 0, sizeof( host4 ) );
    host4.sin_family = AF_INET;
    host4.sin_addr.s_addr = htonl( INADDR_ANY );
    host4.sin_port = htons( port );

    st = bind(idSocket, (struct sockaddr*)&host4, sizeof(host4));

    if (st != 0) {
      throw std::runtime_error( "VSocket::4Bind" );
    }
    
   }

   this -> port = port;
   
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
  int st = listen(this -> idSocket, backlog);

  if (st == -1) {
    throw std::runtime_error("Markpassive");
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
  int st = 0;
  socklen_t addrLen;

  if(this->IPv6) {
    addrLen = sizeof(struct sockaddr_in6);
  } else {
    addrLen = sizeof(struct sockaddr_in);
  }

  if (this->IPv6) {
    sockaddr_in6 host6;
    memset(&host6, 0, sizeof(host6));
    host6.sin6_family = AF_INET6;
    host6.sin6_addr = in6addr_any;
    host6.sin6_port = htons(port);
    st = accept(this->idSocket, (struct sockaddr*)&host6, &addrLen);

  } else {
    sockaddr_in host4;
    memset(&host4, 0, sizeof(host4));
    host4.sin_family = AF_INET;
    host4.sin_addr.s_addr = htonl(INADDR_ANY);
    host4.sin_port = htons(port);
    st = accept(this->idSocket, (struct sockaddr*)&host4, &addrLen);
  }

  if (st == -1) {
    throw std::runtime_error("WaitForConnection");
  }

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
  int st = shutdown(this -> idSocket, mode);
  
  if (st == -1) {
    throw std::runtime_error("Shutdown");
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
  int st = -1;
  socklen_t addrlen;
    
   if (IPv6) {
    addrlen = sizeof(struct sockaddr_in6);
    st = sendto(idSocket, buffer, size, 0, (struct sockaddr*)addr, addrlen);
   } else {
    addrlen = sizeof(struct sockaddr_in);
    st = sendto(idSocket, buffer, size, 0, (struct sockaddr*)addr, addrlen); 
   }


  if (st < 0) {
    throw std::runtime_error( "VSocket::sendTo" );
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
   int st = -1;
   socklen_t addrlen;
    
   if (IPv6) {
    addrlen = sizeof(struct sockaddr_in6);
    st = recvfrom(idSocket, buffer, size, 0, (struct sockaddr*)addr, &addrlen);
   } else {
    addrlen = sizeof(struct sockaddr_in);
    st = recvfrom(idSocket, buffer, size, 0, (struct sockaddr*)addr, &addrlen); 
   }

  if (st < 0) {
    throw std::runtime_error( "VSocket::sendTo" );
  }

  return st;
}

