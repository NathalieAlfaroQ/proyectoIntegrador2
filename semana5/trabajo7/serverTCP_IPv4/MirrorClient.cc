/*
 * Ejemplo de cliente usando sockets (versión Fedora).
 *
 * Deben determinar la dirección IP del equipo donde van a correr el servidor
 * para hacer la conexión en ese punto, con el comando en terminal: ip addr
 *
 * Este programa se conecta a un servidor en una IP específica,
 * envía un mensaje y muestra la respuesta del servidor.
 */

// Bibliotecas
#include <stdio.h>
#include <cstring>

// Encabezados
#include "Socket.h"

#define PORT 1234
#define BUFSIZE 250

int main(int argc, char **argv)
{
   VSocket *s;
   char buffer[BUFSIZE];

   // Crea un socket de tipo stream (TCP) IPv4
   s = new Socket('s', false);
   // Llena el buffer con ceros
   memset(buffer, 0, BUFSIZE);

   /*
    * En terminal, no de VSC, poner: ip addr
    * Tomar la eth0 en inet
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
    */

   // Se conecta al servidor en esa IP y puerto
   s->MakeConnection("172.23.148.148", PORT);

   if (argc > 1)
   {
      // Si se proporciona un argumento, lo envía al servidor
      s->Write(argv[1]);
   }
   else
   {
      // Si no, envía un mensaje por defecto
      s->Write("Hello world 2025 ...");
   }

   // Lee la respuesta del servidor
   s->Read(buffer, BUFSIZE);
   // Imprime la respuesta recibida
   printf("%s", buffer);
}