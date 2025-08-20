#ifndef VSocket_h
#define VSocket_h

class VSocket
{
public:
   void BuildSocket(char, bool = false);
   void BuildSocket(int);

   ~VSocket();
   void Close();

   int EstablishConnection(const char *, int);
   int EstablishConnection(const char *, const char *);
   virtual int MakeConnection(const char *, int) = 0;
   virtual int MakeConnection(const char *, const char *) = 0;

   virtual size_t Read(void *, size_t) = 0;
   virtual size_t Write(const void *, size_t) = 0;
   virtual size_t Write(const char *) = 0;

protected:
   int idSocket;
   bool IPv6;
   int port;  // Puerto asociado al socket.
   char type; // Tipo de socket.
};
#endif // VSocket_h
