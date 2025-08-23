/**
 *   Client side implementation of UDP client-server model
 *
 **/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "VSocket.h"
#include "Socket.h"

#define PORT 1234
#define MAXLINE 1024

int main()
{
   VSocket *client;
   int sockfd;
   int n, len;
   char buffer[MAXLINE];
   char *hello = (char *)"Hello from CI0123 client";
   struct sockaddr_in other;

   // Creates an UDP socket: datagram
   client = new Socket('d');
   memset(&other, 0, sizeof(other));

   other.sin_family = AF_INET;
   other.sin_port = htons(PORT);

   // IP address to test our client with a Python server on lab 3-5
   n = inet_pton(AF_INET, " 172.23.148.148", &other.sin_addr);
   // Poner en la terminar "ip addr" y ver la eth0 en inet antes del /

   n = client->sendTo((void *)hello, strlen(hello), (void *)&other);
   printf("Client: Hello message sent.\n");

   n = client->recvFrom((void *)buffer, MAXLINE, (void *)&other);
   buffer[n] = '\0';
   printf("Client message received: %s\n", buffer);

   client->Close();
   return 0;
}