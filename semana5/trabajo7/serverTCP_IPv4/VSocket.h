/**
 * Interfaz de VSocket, version Fedora.
 **/

// Encabezados
#ifndef VSocket_h
#define VSocket_h

class VSocket
{
public:
   void BuildSocket(char, bool = false);
   void BuildSocket(int);
   ~VSocket();
   
   void createPeer( void *, int );

   void Close();

   int EstablishConnection(const char *, int);
   int EstablishConnection(const char *, const char *);

   virtual int MakeConnection(const char *, int) = 0;
   virtual int MakeConnection(const char *, const char *) = 0;

   virtual size_t Read(void *, size_t) = 0;
   virtual size_t Write(const void *, size_t) = 0;
   virtual size_t Write(const char *) = 0;

   // Asignar una dirección de socket a un descriptor de socket
   int Bind(int);
   // Marcar un socket como pasivo: se usará para aceptar conexiones
   int MarkPassive(int);
   // Esperar una conexión entre pares
   int WaitForConnection(void);

   virtual VSocket *AcceptConnection() = 0;

   /*
    * Provocar que se cierre total o parcialmente una conexión dúplex
    * completa en el socket asociado con el descriptor de archivo.
    */
   int Shutdown(int);

   // UDP metodos
   size_t sendTo(const void *, size_t, void *);
   size_t recvFrom(void *, size_t, void *);

protected:
   int idSocket; // Socket identificador
   bool IPv6;    // IPv6 = true | IPv4 = false
   int port;     // Socket puerto asociado
   char type;    // Socket tipo stream o datagram
};
#endif // VSocket_h