/**
 *
 **/

#include <sys/socket.h>
#include <arpa/inet.h> // ntohs, htons
#include <stdexcept>   // runtime_error
#include <cstring>     // memset
#include <netdb.h>     // getaddrinfo, freeaddrinfo
#include <unistd.h>    // close

#include "VSocket.h"

/**
 *  Constructor.
 *
 *  @param     char t: Tipo de socket.
 *     's' para stream.
 *     'd' para datagram.
 *  @param     bool ipv6: ndica si usa IPv6.
 **/
void VSocket::BuildSocket(char t, bool IPv6)
{
   // Checks if it is an IPv6 and what kind of socket
   this->IPv6 = IPv6;

   // Para IPv6 usa AF_INET6.
   if (this->IPv6)
   {
      // "d" SOCK_DGRAM.
      if (t == 'd')
      {
         this->idSocket = socket(AF_INET6, SOCK_DGRAM, 0);
      }
      else
      {
         this->idSocket = socket(AF_INET6, SOCK_STREAM, 0);
      }
   }
   // Para IPv4 usa AF_INET.
   else
   {
      // "s" SOCK_STREAM.
      if (t == 's')
      {
         this->idSocket = socket(AF_INET, SOCK_STREAM, 0);
      }
      else
      {
         this->idSocket = socket(AF_INET, SOCK_DGRAM, 0);
      }
   }

   // Excepción para error en creación de socket.
   if ((idSocket) == -1)
   {
      throw std::runtime_error("VSocket::BuildSocket, (reason)");
   }
}

/**
 * Class destructor
 **/
VSocket::~VSocket()
{
   this->Close();
}

/**
 * Para cerrar el socket.
 * Usa Unix close system call (Una vez abierto un socket, se maneja como archivo Unix).
 **/
void VSocket::Close()
{
   int connection;
   // Si el socket está abierto, cerrar.

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
 * Conectar a una dirección IP.
 * Usa "connect" Unix system call.
 *
 * @param      char * host: Dirección del host en notación de punto, "10.84.166.62"
 * @param      int port: Dirección del proceso, 80.
 **/
int VSocket::EstablishConnection(const char *hostip, int port)
{
   int connection;

   // Crea conección si es IPv6.
   if (this->IPv6)
   {
      // Estructura con la info del servidor.
      struct sockaddr_in6 server_address6;
      server_address6.sin6_flowinfo = 0;

      // Hacerlo IPv6.
      server_address6.sin6_family = AF_INET6;
      server_address6.sin6_port = htons(port);

      if (inet_pton(AF_INET6, hostip, &server_address6.sin6_addr) <= 0)
      {
         throw std::runtime_error("inet_pton for IPv6");
      }

      // Crear conección
      connection = connect(idSocket, (struct sockaddr *)&server_address6, sizeof(server_address6));
   }
   // Crea conección para IPv4.
   else
   {
      struct sockaddr_in server_address;
      server_address.sin_family = AF_INET;
      server_address.sin_port = htons(port);

      if (inet_pton(AF_INET, hostip, &server_address.sin_addr) <= 0)
      {
         throw std::runtime_error("inet_pton for IPv4");
      }

      connection = connect(idSocket, (struct sockaddr *)&server_address, sizeof(server_address));
   }

   // Lanza excepción si hay error al conectar.
   if (-1 == connection)
   {
      throw std::runtime_error("VSocket::EstablishConnection");
   }

   return connection;
}

/**
 * Conectar a un host por nombre de dominio.
 * Usa "connect" Unix system call.
 *
 * @param      char * host: Dirección del host, tipo punto, "os.ecci.ucr.ac.cr".
 * @param      char * service: Dirección de proceso, "http".
 **/
int VSocket::EstablishConnection(const char *host, const char *service)
{
   int st;
   struct addrinfo hints, *result, *rp;
   memset(&hints, 0, sizeof(struct addrinfo));

   // Permite IPv4 o IPv6.
   hints.ai_family = AF_UNSPEC;
   // Socket tipo stream.
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;

   // Convierte un dominio en direcciones IP.
   st = getaddrinfo(host, service, &hints, &result);

   // Busca que una dirección funcione.
   for (rp = result; rp; rp = rp->ai_next)
   {
      st = connect(idSocket, rp->ai_addr, rp->ai_addrlen);
      if (0 == st)
      {
         break;
      }
   }

   freeaddrinfo(result);
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
int VSocket::Bind(int port)
{
   int st = -1;

   // Creates a connection based on whether it is an IPv6 or not
   if (this->IPv6)
   {
      // Struct that holds the server information
      struct sockaddr_in6 server_address6;
      // Makes it IPv6
      server_address6.sin6_family = AF_INET6;
      server_address6.sin6_addr = in6addr_any;
      server_address6.sin6_port = htons(port);
      // Attempts to create connection
      st = bind(idSocket, (struct sockaddr *)&server_address6, sizeof(server_address6));
   }
   else
   {
      // Same as the if above, except for IPv4
      struct sockaddr_in server_address;
      server_address.sin_family = AF_INET;
      server_address.sin_addr.s_addr = INADDR_ANY;
      server_address.sin_port = htons(port);
      st = bind(idSocket, (struct sockaddr *)&server_address, sizeof(server_address));
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
size_t VSocket::sendTo(const void *buffer, size_t size, void *addr)
{
   int st = -1;
   st = sendto(this->idSocket, buffer, size, 0, (sockaddr *)addr, sizeof(struct sockaddr_in));

   if (st == -1)
   {
      throw std::runtime_error("VSocket::sendTo");
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
size_t VSocket::recvFrom(void *buffer, size_t size, void *addr)
{
   int st = -1;
   socklen_t addr_len = sizeof(struct sockaddr_in);
   st = recvfrom(this->idSocket, buffer, size, 0, (struct sockaddr *)addr, &addr_len);

   if (st == -1)
   {
      throw std::runtime_error("VSocket::recvFrom");
   }

   return st;
}