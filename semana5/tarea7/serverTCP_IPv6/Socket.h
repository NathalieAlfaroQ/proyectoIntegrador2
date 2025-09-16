/*
 * Definición de la interfaz de la clase Socket, versión Fedora.
 */

// Encabezados
#ifndef Socket_h
#define Socket_h

#include "VSocket.h"

/**
 * La clase Socket hereda de VSocket y proporciona métodos para
 * crear, conectar, leer y escribir sockets de una manera más sencilla.
 */
class Socket : public VSocket
{
public:
   // Constructor
   Socket(char, bool = false);
   Socket(int id);
   // Destructor
   ~Socket();

   // Conexión usando IP
   int MakeConnection(const char *, int);
   // Conexión usando DNS
   int MakeConnection(const char *, const char *);

   // Leer datos
   size_t Read(void *, size_t);
   // Escribir datos
   size_t Write(const void *, size_t);
   // Escribir texto
   size_t Write(const char *);

   // Aceptar conexiones
   VSocket *AcceptConnection();

protected:
   //int idSocket;
};
#endif // Socket_h