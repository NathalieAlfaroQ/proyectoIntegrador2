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
#include <arpa/inet.h>

#include <stdexcept>

#include "SSLSocket.h"
#include "VSocket.h"

#define FAIL    -1

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

   this->Init(false);					// Initializes to client context

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
SSLSocket::SSLSocket(const char * certFileName,const char * keyFileName, bool IPv6 ) {
   this->BuildSocket( 's', IPv6 );

   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;

   // Add SSL functions and context
   this->Init(true);

   // Load certificates
   this->LoadCertificates(certFileName, keyFileName);
}


/**
  *  Class constructor
  *
  *  @param     int id: socket descriptor
  *
 **/
SSLSocket::SSLSocket( int id ) {
    this->idSocket = id;
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

   ssl = SSL_new(reinterpret_cast<SSL_CTX *>(this->SSLContext));
   if (ssl == NULL) {
      throw std::runtime_error("SSLSocket::Init: Failed to create the SSL object\n");
   }

   this->SSLStruct = (void*)ssl;

}


/**
  *  InitContext
  *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings, TLS_server_method, SSL_CTX_new
  *
  *  Creates a new SSL server context to start encrypted comunications, this context is stored in class instance
  *
 **/
void SSLSocket::InitContext( bool serverContext ) {
   SSL_library_init();
   OpenSSL_add_all_algorithms();
   SSL_load_error_strings();
   const SSL_METHOD * method;
   if ( serverContext ) {
      method = TLS_server_method();
   } else {
      method = TLS_client_method();
   }

   // No method created.
   if ( nullptr == method ) {
      throw std::runtime_error( "SSLSocket::InitContext( bool ): unable to create method" );
   }

   SSL_CTX * context = SSL_CTX_new(method);

   if ( nullptr == context ) {
      throw std::runtime_error( "SSLSocket::InitContext( bool ): unable to create context" );
   }

   // Verify server's certificates
   SSL_CTX_set_verify(context, SSL_VERIFY_NONE, NULL);

   // Default trusted certificate path
   if (!SSL_CTX_set_default_verify_paths(context)) {
      throw std::runtime_error("SSLSocket::InitContext( bool ): Failed to set the default trusted certificate store\n");
  }

   this->SSLContext = (SSL_CTX*)context;
}


/**
 *  Load certificates
 *    verify and load certificates
 *
 *  @param	const char * certFileName, file containing certificate
 *  @param	const char * keyFileName, file containing keys
 *
 **/
 void SSLSocket::LoadCertificates( const char * CertFile, const char * KeyFile ) {
   /* set the local certificate from CertFile */
   if ( SSL_CTX_use_certificate_file( (SSL_CTX*)this->SSLContext, CertFile, SSL_FILETYPE_PEM ) <= 0 ) {
      ERR_print_errors_fp( stderr );
      abort();
   }
   /* set the private key from KeyFile (may be the same as CertFile) */
   if ( SSL_CTX_use_PrivateKey_file( (SSL_CTX*)this->SSLContext, KeyFile, SSL_FILETYPE_PEM ) <= 0 ) {
      ERR_print_errors_fp( stderr );
      abort();
   }
   /* verify private key */
   if ( ! SSL_CTX_check_private_key( (SSL_CTX*)this->SSLContext ) ) {
      fprintf( stderr, "Private key does not match the public certificate\n" );
      abort();
   }
   printf("Finish: LoadCertificates()");
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

   SSL_set_fd(reinterpret_cast<SSL *>(this->SSLStruct), this->idSocket);
   if (SSL_connect(reinterpret_cast<SSL *>(this->SSLStruct)) != 1) {
      throw std::runtime_error("SSLSocket::MakeConnection(const char *, const char *): SSL_connect failed");
   }

   return st;

}

/**
 *   SSLSocket::Accept
 *
 *  waits for a TLS/SSL client to initiate the TLS/SSL handshake
 *
 **/
VSocket * SSLSocket::AcceptConnection(){
   struct sockaddr_in addr;
   socklen_t len = sizeof( addr );

   // Accept TCP connection
   int client_fd = accept(this->idSocket, (struct sockaddr*) &addr, &len );
   if (client_fd == -1) {
      throw std::runtime_error("SSLSocket::AcceptConnection(): failed to accept()");
   }

   SSLSocket *client_socket = new SSLSocket(client_fd);

   // Copy SSL context from server to client socket
   client_socket->Copy(this);

   // Set the file descriptor for the CLIENT's SSL structure
   SSL_set_fd(reinterpret_cast<SSL *>(client_socket->SSLStruct), client_fd);

   int st = SSL_accept(reinterpret_cast<SSL *>(client_socket->SSLStruct));
   if (st == FAIL) {
      // Get more detailed error information
      int ssl_error = SSL_get_error(reinterpret_cast<SSL *>(client_socket->SSLStruct), st);
      fprintf(stderr, "SSL_accept failed with error: %d\n", ssl_error);
      ERR_print_errors_fp(stderr);

      delete client_socket;
      throw std::runtime_error("SSLSocket::AcceptConnection(): SSL_accept failed");
   }

   printf("SSL handshake completed successfully with client\n");
   return client_socket;
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

   SSL_set_fd(reinterpret_cast<SSL *>(this->SSLStruct), this->idSocket);
   if (SSL_connect(reinterpret_cast<SSL *>(this->SSLStruct)) != 1) {
      throw std::runtime_error("SSLSocket::MakeConnection(const char *, const char *): SSL_connect failed");
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
size_t SSLSocket::Read( void * buffer, size_t max_size ) {
   int bytes = SSL_read(reinterpret_cast<SSL*>(this->SSLStruct), buffer, max_size);
   
   if (bytes > 0) {
      return bytes;
   } else if (bytes == 0) {
      // Connection closed
      return 0;
   } else {
      int ssl_error = SSL_get_error(reinterpret_cast<SSL*>(this->SSLStruct), bytes);
      if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
         return 0;  // No data available now
      } else {
         throw std::runtime_error("SSLSocket::Read: SSL_read failed");
      }
   }
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
   if (string == nullptr) {
      throw std::runtime_error("SSLSocket::Write: null pointer");
   }
   return this->Write(string, strlen(string));
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
size_t SSLSocket::Write( const void * buffer, size_t size ) {

   int bytes = SSL_write((SSL*)SSLStruct, buffer, size);

   return bytes;
}


/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts(bool server) {
   X509 *cert;
   char *line;

   // Show our own certificate
   if (server) {
      cert = SSL_get_certificate( (SSL *) this->SSLStruct );
      if ( nullptr != cert ) {
         printf("Our certificates:\n");
         line = X509_NAME_oneline( X509_get_subject_name( cert ), 0, 0 );
         printf( "Subject: %s\n", line );
         free( line );
         line = X509_NAME_oneline( X509_get_issuer_name( cert ), 0, 0 );
         printf( "Issuer: %s\n", line );
         free( line );
      } else {
         printf( "No certificates owned.\n" );
      }
   } else {
      // Show peer certificate
      cert = SSL_get_peer_certificate( (SSL *) this->SSLStruct );
      if ( nullptr != cert ) {
         printf("Peer certificates:\n");
         line = X509_NAME_oneline( X509_get_subject_name( cert ), 0, 0 );
         printf( "Subject: %s\n", line );
         free( line );
         line = X509_NAME_oneline( X509_get_issuer_name( cert ), 0, 0 );
         printf( "Issuer: %s\n", line );
         free( line );
         X509_free( cert );
      } else {
         printf( "No peer certificates.\n" );
      }
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
void SSLSocket::Copy(SSLSocket *original) {

   int st;

   // Constructs a new SSL socket
   SSL *ssl = SSL_new((SSL_CTX*)original->SSLContext);
   if (ssl == nullptr) {
      throw std::runtime_error("SSL_new failed in SSLSocket::Copy");
   }

   // Assign new variable to instance variable
   this->SSLStruct = (void*)ssl;

   // Change connection status to SSL using SSL_set_fd() function
   st = SSL_set_fd(reinterpret_cast<SSL *>(this->SSLStruct), this->idSocket);
   if (st != 1) {
      SSL_free(reinterpret_cast<SSL *>(this->SSLStruct));
      this->SSLStruct = nullptr;
      throw std::runtime_error("SSL_set_fd failed in SSLSocket::Copy");
   }

   // Optional: Copy other relevant members if needed
   this->SSLContext = original->SSLContext;
   this->IPv6 = original->IPv6;
}