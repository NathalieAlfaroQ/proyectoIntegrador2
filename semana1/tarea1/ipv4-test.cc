/*
 * Ejemplo de cómo usar sockets en IPv4 para
 * conectarse a un servidor web y enviar una petición HTTP.
 *
 * Crea un socket TCP.
 * Se conecta al servidor en 10.84.166.62:80.
 * Envía una petición HTTP pidiendo información.
 * Recibe la respuesta del servidor.
 * Imprime la respuesta en pantalla.
 */

#include <stdio.h>  // printf
#include <string.h> // strlen para manipular cadenas de texto.

// Encabezados
#include "VSocket.h"
#include "Socket.h"

int main(int argc, char *argv[])
{
   // URL de un servidor web de la UCR.
   const char *os = "http://os.ecci.ucr.ac.cr/";

   // Dirección IP de un servidor local.
   const char *osi = "10.84.166.62";

   // Dirección IP de otro servidor de la UCR.
   const char *ose = "163.178.104.62";

   /*
    * GET /pirofs/index.php?disk=Disk-01&cmd=ls HTTP/1.1 Solicita una página del servidor.
    * host: redes.ecci Dominio del servidor.
    * \r\n\r\n Final de la petición.
    */
   const char *request = "GET /pirofs/index.php?disk=Disk-01&cmd=ls HTTP/1.1\r\nhost: redes.ecci\r\n\r\n";

   // Puntero a un socket.
   VSocket *s;

   // Crea un buffer de 512 bytes para recibir datos.
   char a[512];

   // Crea un socket de tipo stream (TCP) y "s" apunta al socket.
   s = new Socket('s');

   // Para conectar el socket al servidor en la IP de osi y el puerto 80 que es de HTTP.
   s->MakeConnection(osi, 80);

   // Envía la cadena request al servidor
   s->Write(request);

   // Lee hasta 512 bytes de datos desde el socket y los almacena en "a".
   s->Read(a, 512);

   // Imprime el contenido del buffer "a", que contiene la respuesta del servidor.
   printf("%s\n", a);
} // Fin main