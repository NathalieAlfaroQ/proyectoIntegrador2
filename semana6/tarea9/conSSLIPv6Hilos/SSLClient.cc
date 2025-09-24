/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  ****** SSLSocket example 
  *
  * (Fedora version)
  *
 **/

#include <cstdlib>
#include <cstring>	// strlen
#include <cstdio>

#include "SSLSocket.h"

/**
 * Main program
 **/
int main(int cuantos, char * argumentos[] ) {
   // Check arguments
   if ( cuantos != 3 ) {
      printf("usage: %s <hostname> <portnum>\n", argumentos[0] );
      exit(0);
   }

   // Create the SSL client socket
   SSLSocket *client = new SSLSocket(true); // TRUE para indicar que usa IPv6
   // Make the connection
   char *hostname, *service;
   hostname = argumentos[ 1 ];
   service = argumentos[ 2 ];
   client->MakeConnection( hostname, service );

   // Get user input
   printf( "Enter the User Name : " );
   char userName[16] = { 0 };
   scanf( "%s", userName );
   printf( "\nEnter the Password : " );
   char password[16] = { 0 };
   scanf( "%s", password );

   // Show encryption and certs
   printf( "\n\nConnected with %s encryption\n", client->GetCipher() );
   client->ShowCerts(false);		// display any certs

   // Create the request message
   char clientRequest[ 1024 ] = { 0 };
   const char * requestMessage =
      "\n<Body>\n"
      "\t<UserName>%s</UserName>\n"
      "\t<Password>%s</Password>\n"
      "</Body>\n";
   sprintf( clientRequest, requestMessage, userName, password );	// construct reply
   client->Write( clientRequest );		// encrypt & send message
   printf("Request sent: %s\n", clientRequest);

   // Get reply
   char buf[1024];
   printf("Waiting for response ... \n");
   int bytes = client->Read( buf, sizeof( buf ) );			// get reply & decrypt
   buf[ bytes ] = 0;
   printf("Received: \"%s\"\n", buf);

   return 0;

}

