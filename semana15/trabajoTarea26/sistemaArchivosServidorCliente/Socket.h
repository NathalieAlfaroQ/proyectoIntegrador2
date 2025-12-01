

#ifndef Socket_h
#define Socket_h
#include "VSocket.h"

class Socket : public VSocket {

   public:
      Socket( char, bool = false );
      Socket( int id );
      ~Socket();
      int MakeConnection( const char *, int );
      int MakeConnection( const char *, const char * );
      size_t Read( void *, size_t );
      size_t Write( const void *, size_t );
      size_t Write( const char * );

      VSocket * AcceptConnection();
      
   protected:

};

#endif

