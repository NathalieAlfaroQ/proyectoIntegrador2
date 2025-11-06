/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *  Socket class implementation
  *
  * (Fedora version)
  *
 **/
 
// SSL includes
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <iostream>
#include <stdexcept>

#include "SSLSocket.h"
#include "Socket.h"

/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket( bool IPv6 ) {

   this->BuildSocket( 's', IPv6 );

   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;

   this->Init();					// Initializes to client context

}


/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool IPv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket( const char * certFileName, const char * keyFileName, bool IPv6 ) {

   this->BuildSocket( 's', IPv6 );

   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;

   this->Init(true);					// Initializes to client context
   this->LoadCertificates(certFileName, keyFileName);
}


/**
  *  Class constructor
  *
  *  @param     int id: socket descriptor
  *
 **/
SSLSocket::SSLSocket( int id ) {

   this->BuildSocket( id );

}


/**
  * Class destructor
  *
 **/
SSLSocket::~SSLSocket() {

// SSL destroy
   if ( nullptr != this->SSLContext ) {
      SSL_CTX_free( reinterpret_cast<SSL_CTX *>( this->SSLContext ) );
   }
   if ( nullptr != this->SSLStruct ) {
      SSL_shutdown(reinterpret_cast<SSL *>(this->SSLStruct));
      SSL_free( reinterpret_cast<SSL *>( this->SSLStruct ) );
   }

   this->Close();

}


/**
  *  SSLInit
  *     use SSL_new with a defined context
  *
  *  Create a SSL object
  *
 **/
void SSLSocket::Init( bool serverContext ) {
   SSL * ssl = nullptr;

   this->InitContext( serverContext );
   ssl = SSL_new( (SSL_CTX *) SSLContext);

   if ( nullptr == ssl ) {
   throw std::runtime_error( "SSLSocket::Init( bool )" );
   }

   this->SSLStruct = (void *) ssl;
}


/**
  *  InitContext
  *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings, TLS_server_method, SSL_CTX_new
  *
  *  Creates a new SSL server context to start encrypted comunications, this context is stored in class instance
  *
 **/
void SSLSocket::InitContext( bool serverContext ) {
   const SSL_METHOD * method;
   SSL_CTX * context;

   if ( serverContext ) {
    method = TLS_server_method();
   } else {
    method = TLS_client_method();
   }

   if ( nullptr == method ) {
      throw std::runtime_error( "SSLSocket::InitContext( bool )" );
   }

   context = SSL_CTX_new(method);

   if ( nullptr == context ) {
      throw std::runtime_error( "SSLSocket::InitContext( bool )" );
   }

   SSLContext = (void *) context;
}


/**
 *  Load certificates
 *    verify and load certificates
 *
 *  @param	const char * certFileName, file containing certificate
 *  @param	const char * keyFileName, file containing keys
 *
 **/
 void SSLSocket::LoadCertificates( const char * certFileName, const char * keyFileName ) {
  int st = -1;

  st = SSL_CTX_use_certificate_file((SSL_CTX *)SSLContext, certFileName,  SSL_FILETYPE_PEM);

  if ( 1 != st ) {
    throw std::runtime_error( "SSLSocket::LoadCertificatesCertificates( const char * certFileName, const char * keyFileName )" );
   }

  st = SSL_CTX_use_PrivateKey_file((SSL_CTX *)SSLContext, keyFileName,  SSL_FILETYPE_PEM);

  if ( 1 != st ) {
    throw std::runtime_error( "SSLSocket::LoadCertificatesKey( const char * certFileName, const char * keyFileName )" );
  }

  st = SSL_CTX_check_private_key((SSL_CTX *)SSLContext);

  if ( 1 != st) {
    throw std::runtime_error("SSLSocket::LoadCertificates - certificate and key do not match");
  }
}
 

/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	int port, service number
 *
 **/
int SSLSocket::MakeConnection( const char * hostName, int port ) {
   int st;

   st = this->EstablishConnection( hostName, port );		// Establish a non ssl connection first
   st = SSL_set_fd((SSL*)SSLStruct, idSocket);

   if ( 1 != st ) {
      throw std::runtime_error( "SSLSocket:MakeConnectionFD( const char * hostName, int port )" );
   }

   st = SSL_connect((SSL*)SSLStruct);

   if ( 1 != st ) {
      throw std::runtime_error( "SSLSocket:MakeConnectionCO( const char * hostName, int port )" );
   }

   return st;

}


/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	char * service, service name
 *
 **/
int SSLSocket::MakeConnection( const char * host, const char * service ) {
   int st;

   st = this->EstablishConnection( host, service );

   st = SSL_set_fd((SSL*)SSLStruct, idSocket);

   if ( 1 != st ) {
      throw std::runtime_error( "SSLSocket:MakeConnectionFd( const char * hostName, int port )" );
   }

   st = SSL_connect((SSL*)SSLStruct);

   if ( 1 != st ) {
      throw std::runtime_error( "SSLSocket:MakeConnectionCo( const char * hostName, int port )" );
   }

   return st;

}


/**
  *  Read
  *     use SSL_read to read data from an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity read
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Read( void * buffer, size_t size ) {
   size_t st = SSL_read((SSL*)SSLStruct, buffer, size);

   if (st <= 0) {
      throw std::runtime_error("SSLSocket::Read() failed");
   }

   return st;
}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Writes data to a secure channel
  *
 **/
size_t SSLSocket::Write( const char * string ) {
   return Write(static_cast<const void*>(string), strlen(string));
}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Write(const void* buffer, size_t size) {
   size_t st = SSL_write((SSL*)SSLStruct, buffer, size);

   if (st <= 0) {
      throw std::runtime_error("SSLSocket::write() failed");
   }

   return st;
}

/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts() {
   X509 *cert;
   char *line;

   cert = SSL_get_peer_certificate( (SSL *) this->SSLStruct );		 // Get certificates (if available)
   if ( nullptr != cert ) {
      printf("Server certificates:\n");
      line = X509_NAME_oneline( X509_get_subject_name( cert ), 0, 0 );
      printf( "Subject: %s\n", line );
      free( line );
      line = X509_NAME_oneline( X509_get_issuer_name( cert ), 0, 0 );
      printf( "Issuer: %s\n", line );
      free( line );
      X509_free( cert );
   } else {
      printf( "No certificates.\n" );
   }

}


/**
 *   Return the name of the currently used cipher
 *
 **/
const char * SSLSocket::GetCipher() {

   return SSL_get_cipher( reinterpret_cast<SSL *>( this->SSLStruct ) );

}

/**
 *   Create constructs a new SSL * variable from a previous created context
 *
 *  @param	Socket * original socket with a previous created context
 *
 **/
void SSLSocket::Copy( SSLSocket * original ) {
   SSL * ssl = nullptr;
   int st;

   ssl = SSL_new((SSL_CTX *)(original -> SSLContext));

   if ( nullptr == ssl ) {
      throw std::runtime_error( "SSLSocket::Copy( SSLSocket*)" );
   }

   st = SSL_set_fd(ssl, idSocket);

   if ( 1 != st ) {
      throw std::runtime_error( "SSLSocket::Copy( SSLSocket*)" );
   }

   SSLStruct = (void*) ssl;
}

/**
 *   SSLSocket::AcceptConnection
 *
 *  waits for a TLS/SSL client to initiate the TLS/SSL handshake
 *
 **/
void SSLSocket::AcceptSSL(){
   int st = -1;

   st = SSL_accept((SSL*)SSLStruct);

   if ( 1 != st ) {
      throw std::runtime_error( "SSLSocket::AcceptSSL()" );
   }
}

VSocket * SSLSocket::AcceptConnection(){
   int id;
   SSLSocket * peer;

   id = this->WaitForConnection();

   peer = new SSLSocket(id);

   peer->Copy(this);

   peer->AcceptSSL();

   return peer;
}


