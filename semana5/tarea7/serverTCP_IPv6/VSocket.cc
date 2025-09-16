/**
 * Implementacion de la clase VSocket, version Fedora.
 * Creacion y  gestion de sockets.
 * Soporta IPv4, IPv6, UDP y TCP.
 **/

// Bibliotecas
#include <sys/socket.h>
#include <arpa/inet.h> // ntohs, htons
#include <stdexcept>   // runtime_error
#include <cstring>     // memset
#include <netdb.h>     // getaddrinfo, freeaddrinfo
#include <unistd.h>    // close

// Encabezados
#include "VSocket.h"

using namespace std;

/**
 * Clase contructor.
 * Crea un socket de tipo s = stream | d = datagram
 * Se elige si tipo IPv4 o IPv6.
 **/
void VSocket::BuildSocket(char t, bool IPv6)
{
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

   // En caso de error se notifica
   if ((idSocket) == -1)
   {
      throw runtime_error("VSocket::BuildSocket");
   }
}

void VSocket::BuildSocket( int id ){

  this->idSocket = id;

}

/**
 * Destructor.
 **/
VSocket::~VSocket()
{
   this->Close();
}

/**
 * Cierra el socket si esta abierto.
 **/
void VSocket::Close()
{
   int connection;

   if (this->idSocket != -1)
   {
      connection = close(this->idSocket);

      if (connection == -1)
      {
         throw runtime_error("VSocket::Close");
      }
      this->idSocket = -1;
   }
}

/**
 * Establece conexión con IP y puerto (modo cliente).
 **/
int VSocket::EstablishConnection(const char *hostip, int port)
{
   int connection;

   // Crea una conexion
   if (this->IPv6)
   {
      // Estructura que contiene la información del servidor
      struct sockaddr_in6 server_address6;
      server_address6.sin6_flowinfo = 0;
      // Lo hace IPv6
      server_address6.sin6_family = AF_INET6;
      server_address6.sin6_port = htons(port);
      // Pasa la dirección a binario

      if (inet_pton(AF_INET6, hostip, &server_address6.sin6_addr) <= 0)
      {
         throw runtime_error("inet_pton for IPv6");
      }

      // Intentos de crear conexión
      connection = connect(idSocket, (struct sockaddr *)&server_address6, sizeof(server_address6));
   }
   else
   {
      // Ahora para IPv4
      struct sockaddr_in server_address;
      server_address.sin_family = AF_INET;
      server_address.sin_port = htons(port);

      if (inet_pton(AF_INET, hostip, &server_address.sin_addr) <= 0)
      {
         throw runtime_error("inet_pton for IPv4");
      }

      connection = connect(idSocket, (struct sockaddr *)&server_address, sizeof(server_address));
   }

   // Si hay error, notifica
   if (-1 == connection)
   {
      throw runtime_error("VSocket::EstablishConnection");
   }

   return connection;
}

/**
 * Establece conexión mediante nombre DNS y servicio.
 **/
int VSocket::EstablishConnection(const char *host, const char *service)
{
   int st;
   struct addrinfo hints, *result, *rp;

   memset(&hints, 0, sizeof(struct addrinfo));
   // Soporta IPv4 o IPv6
   hints.ai_family = AF_UNSPEC;
   // Stream socket
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;

   st = getaddrinfo(host, service, &hints, &result);

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
 * Enlaza el socket a un puerto (modo servidor).
 **/
int VSocket::Bind(int port)
{
   int st = -1;

   // Crea conexion IPv6
   if (this->IPv6)
   {
      // Estructura que contiene la información del servidor
      struct sockaddr_in6 server_address6;
      // Lo hace IPv6
      server_address6.sin6_family = AF_INET6;
      server_address6.sin6_addr = in6addr_any;
      server_address6.sin6_port = htons(port);
      // Intentos de vinculación
      st = bind(idSocket, (struct sockaddr *)&server_address6, sizeof(server_address6));
   }
   else
   {
      // Lo mismo pero para IPv4
      struct sockaddr_in server_address;
      server_address.sin_family = AF_INET;
      server_address.sin_addr.s_addr = INADDR_ANY;
      server_address.sin_port = htons(port);
      st = bind(idSocket, (struct sockaddr *)&server_address, sizeof(server_address));
   }

   // Si hay error, notifique
   if (-1 == st)
   {
      throw runtime_error("VSocket::Bind2025");
   }

   return st;
}

/**
 * Marca el socket como pasivo (modo servidor).
 **/
int VSocket::MarkPassive(int backlog)
{
   int st = -1;
   st = listen(idSocket, backlog);

   if (-1 == st)
   {
      throw runtime_error("VSocket::MarkPassive");
   }

   return st;
}

/**
 * Espera y acepta una conexión entrante (modo servidor).
 **/
int VSocket::WaitForConnection(void)
{
   int st = -1;
   st = accept(idSocket, nullptr, nullptr);

   if (-1 == st)
   {
      throw runtime_error("VSocket::WaitForConnection");
   }

   return st;
}

/**
 * Apaga el canal de comunicación del socket.
 **/
int VSocket::Shutdown(int mode)
{
   int st = -1;
   st = shutdown(idSocket, mode);

   if (-1 == st)
   {
      throw runtime_error("VSocket::Shutdown");
   }

   return st;
}

// UDP 2025

// Enviar datos
size_t VSocket::sendTo(const void *buffer, size_t size, void *addr)
{
   int st = -1;
   st = sendto(this->idSocket, buffer, size, 0, (sockaddr *)addr, sizeof(struct sockaddr_in));

   if (st == -1)
   {
      throw runtime_error("VSocket::sendTo");
   }

   return st;
}

// Recibe datos
size_t VSocket::recvFrom(void *buffer, size_t size, void *addr)
{
   int st = -1;
   socklen_t addr_len = sizeof(struct sockaddr_in);
   st = recvfrom(this->idSocket, buffer, size, 0, (struct sockaddr *)addr, &addr_len);

   if (st == -1)
   {
      throw runtime_error("VSocket::recvFrom");
   }

   return st;
}