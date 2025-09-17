/*
 * Ejemplo de cliente usando sockets.
 *
 * Este programa se conecta a un servidor en una IP específica,
 * envía un mensaje y muestra la respuesta del servidor.
 */

// Bibliotecas
#include <stdio.h>
#include <cstring>
#include <iostream>

// Encabezados
#include "Socket.h"
#include "lector_figura.h"

// Puerto a utilizar
#define PORT 1234
// Tamaño del buffer
#define BUFSIZE 2048

int main(int argc, char **argv)
{
   VSocket *s;
   // Se crea un buffet de tipo char de tamaño 2048
   char buffer[BUFSIZE];
   // Crea un socket de tipo stream (TCP) IPv4
   s = new Socket('s', false);
   // Llena el buffer con ceros
   memset(buffer, 0, BUFSIZE);

   /*
    * En terminal poner: ip addr
    * Tomar la eth0 en inet del 1 hasta el /
    *
    * alfanath@Alfa:~$ ip addr
    * 1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    * link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    *  inet 127.0.0.1/8 scope host lo
    *     valid_lft forever preferred_lft forever
    * inet6 ::1/128 scope host
    *   valid_lft forever preferred_lft forever
    * 2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP group default qlen 1000
    * link/ether 00:15:5d:f6:1e:ed brd ff:ff:ff:ff:ff:ff
    * inet 172.23.148.148/20 brd 172.23.159.255 scope global eth0
    *    valid_lft forever preferred_lft forever
    * inet6 fe80::215:5dff:fef6:1eed/64 scope link
    *   valid_lft forever preferred_lft forever
    * 
    */

   // Se conecta al servidor en esa IP y puerto
   s->MakeConnection("172.23.148.148", PORT);

   // Le consulta al usuario por la figura que desea
   std::string figura;
   std::cout << "Digite el nombre de la figura: ";
   std::cin >> figura;

   // Envía al servidor el nombre de la figura
   // figura.c_str() convierte el string en un arreglo de char terminado en \0, que es lo que espera el socket
   s->Write(figura.c_str());

   // Espera la respuesta del servidor, la cual llega a buffer y se puede imprimir
   s->Read(buffer, BUFSIZE);
   std::cout << "Figura recibida:\n" << buffer << std::endl;

   // Termina la conexión con el servidor
   s->Close();
   
   //Termina el programa
   return 0;
}