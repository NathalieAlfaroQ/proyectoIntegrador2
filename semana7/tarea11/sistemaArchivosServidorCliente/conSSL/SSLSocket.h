/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *   SSL Socket class interface
  *
  * (Fedora version)
  *
 **/

#ifndef SSLSocket_h
#define SSLSocket_h

#include "VSocket.h"


class SSLSocket : public VSocket {

   public:
      SSLSocket(const char *, const char *, bool = false );
      SSLSocket( bool IPv6 = false );				// Not possible to create with UDP, client constructor
      SSLSocket( int );
      ~SSLSocket();
      int MakeConnection( const char *, int );
      int MakeConnection( const char *, const char * );
      VSocket * AcceptConnection() override;
      void InitBio(const char* host);
      size_t Write( const char * );
      size_t Write( const void *, size_t );
      size_t Read( void *, size_t );
      void ShowCerts(bool server);
      const char * GetCipher();
      void Copy( SSLSocket * original );

   private:
      void Init( bool = false );		// Defaults to create a client context, true if server context needed
      void InitContext( bool );
      void LoadCertificates( const char *, const char * );

// Instance variables
      void* SSLContext;				// SSL context
      void* SSLStruct;					// SSL Socket

};

#endif

