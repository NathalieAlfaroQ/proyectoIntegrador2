/*
 *  Esta prueba solo funciona utilizando un equipo de la red interna de la ECCI, por lo que
 *  deberan realizarlo en la ECCI o  conectarse por la VPN para completarla.
 *  La direccion IPv6 provista es una direccion privada.
 *  Tambien deben prestar atencion al componente que esta luego del "%" en la direccion y que hace
 *  referencia a la interfaz de red utilizada para la conectividad, en el ejemplo se presenta la interfaz "eno1"
 *  pero es posible que su equipo tenga otra interfaz.
 *
 *  Ejemplo de conexión mediante sockets en IPv6 para hacer una petición HTTP a un
 *  servidor dentro de la red interna de la ECCI.
 *
 * Crea un socket IPv6.
 * Se conecta al servidor IPv6 en fe80::215:5dff:fe01:a7ed%eth0:8080.
 * Envía una petición HTTP para obtener la página principal.
 * Recibe la respuesta del servidor.
 * Imprime la respuesta en pantalla.
 */

#include <stdio.h>  // printf
#include <string.h> // memset inicializa la memoria del buffer.

// Encabezados
#include "Socket.h"

int main(int argc, char *argv[])
{
   // Dirección IPv6 privada del servidor con la interfaz eth0.
  // const char *lab = "fe80::215:5dff:fe01:a7ed%eth0";

   //const char * lab = "fe80::8f5a:e2e1:7256:ffe3%enp0s31f6";
      //const char * lab = "fe80::8f5a:e2e1:7256:ffe3%eth0";
   const char * lab = "fe80::8f5a:e2e1:7256:ffe3%enp0s31f6";


   /*
    * Petición HTTP que se enviará al servidor.
    *
    * "GET / HTTP/1.1" Pide la página principal.
    * "host: redes.ecci" Nombre del servidor.
    *  \r\n\r\n Finaliza la petición.
    */
   const char *request = (char *) "GET /aArt/index.php?disk=Disk-01&fig=whale-1.txt\r\nHTTP/v1.1\r\nhost: redes.ecci\r\n\r\n";


   // Crea un socket TCP llamado "s" en IPv6 que se especifica con true.
   Socket s('s', true);

   // Buffer de 512 bytes, almacena la respuesta del servidor.
   char a[512];

   // Llena el buffer con 0.
   memset(a, 0, 512);

   // Para conectarse al servidor IPv6 en el puerto 8080.
   s.MakeConnection(lab, (char *)"http");

   // Envía la petición HTTP al servidor.
   s.Write(request);

   // Lee hasta 512 bytes desde el socket y los almacena en "a".
   s.Read(a, 512);
   printf("%s\n", a);
} // Fin main