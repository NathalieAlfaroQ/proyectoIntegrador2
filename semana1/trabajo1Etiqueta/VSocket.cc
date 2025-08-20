#include <sys/socket.h>
#include <arpa/inet.h> // ntohs, htons
#include <stdexcept>   // runtime_error
#include <cstring>     // memset
#include <netdb.h>     // getaddrinfo, freeaddrinfo
#include <unistd.h>    // close

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
void VSocket::BuildSocket(char t, bool IPv6)
{
   // Checks if it is an IPv6 and what kind of socket
   this->IPv6 = IPv6;

   if (this->IPv6)
   {
      if (t == 'd')
      {
         this->idSocket = socket(AF_INET6, SOCK_DGRAM, 0);
      }
      else
      {
         this->idSocket = socket(AF_INET6, SOCK_STREAM, 0);
      }
   }
   else
   {
      if (t == 's')
      {
         this->idSocket = socket(AF_INET, SOCK_STREAM, 0);
      }
      else
      {
         this->idSocket = socket(AF_INET, SOCK_DGRAM, 0);
      }
   }

   // In case it is done incorrectly, print an error message.
   if ((idSocket) == -1)
   {
      throw std::runtime_error("VSocket::BuildSocket, (reason)");
   }
}

/**
 * Class destructor
 *
 **/
VSocket::~VSocket()
{

   this->Close();
}

/**
 * Close method
 *    use Unix close system call (once opened a socket is managed like a file in Unix)
 *
 **/
void VSocket::Close()
{
   int connection;
   // If the socket is open, close it
   if (this->idSocket != -1)
   {
      connection = close(this->idSocket);
      if (connection == -1)
      {
         throw std::runtime_error("Socket::Close");
      }
      this->idSocket = -1;
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
int VSocket::EstablishConnection(const char *hostip, int port)
{

   int connection;

   // Creates a connection based on whether it is an IPv6 or not
   if (this->IPv6)
   {
      // Struct that holds the server information
      struct sockaddr_in6 server_address6;
      server_address6.sin6_flowinfo = 0;
      // Makes it IPv6
      server_address6.sin6_family = AF_INET6;
      server_address6.sin6_port = htons(port);
      // Passes the address into binary
      if (inet_pton(AF_INET6, hostip, &server_address6.sin6_addr) <= 0)
      {
         perror("inet_pton for IPv6");
         exit(EXIT_FAILURE);
      }
      // Attempts to create connection
      connection = connect(idSocket, (struct sockaddr *)&server_address6, sizeof(server_address6));
   }
   else
   {
      // Same as the if above, except for IPv4
      struct sockaddr_in server_address;
      server_address.sin_family = AF_INET;
      server_address.sin_port = htons(port);
      if (inet_pton(AF_INET, hostip, &server_address.sin_addr) <= 0)
      {
         perror("inet_pton for IPv4");
         exit(EXIT_FAILURE);
      }

      connection = connect(idSocket, (struct sockaddr *)&server_address, sizeof(server_address));
   }

   if (-1 == connection)
   {
      throw std::runtime_error("VSocket::EstablishConnection");
   }
   return connection;
}

/**
 * EstablishConnection method
 *   use "connect" Unix system call
 *
 * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
 * @param      char * service: process address, example "http"
 *
 **/
int VSocket::EstablishConnection(const char *host, const char *service)
{
   int st;
   struct addrinfo hints, *result, *rp;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;
   st = getaddrinfo(host, service, &hints, &result);

   for (rp = result; rp; rp = rp->ai_next)
   {
      st = connect(idSocket, rp->ai_addr, rp->ai_addrlen);
      if (0 == st)
         break;
   }

   freeaddrinfo(result);
   return st;
}