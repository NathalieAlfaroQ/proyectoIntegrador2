/**
 * Cliente TCP IPv4 normal o SSL según los parámetros de entrada.
 **/

#include <stdio.h>
#include <string.h>

#include "VSocket.h"
#include "Socket.h"
#include "SSLSocket.h"

// Tamaño máximo del buffer de lectura
#define MAXBUF 1024

int main(int argc, char *argv[])
{
    VSocket *client; // Puntero a un objeto de tipo VSocket 
    int st, port = 80; // Se inicializa el puerto a 80 (HTTP)
    char a[MAXBUF]; // Buffer para almacenar la respuesta del servidor
    
    // Dirección IP del servidor con el que se quiere conectar
    char *os = (char *)"163.178.104.62";
    
    // Petición HTTP que se enviará al servidor
    char *whale = (char *)"GET /aArt/index.php?disk=Disk-01&fig=whale-1.txt\r\nHTTP/v1.1\r\nhost: redes.ecci\r\n\r\n";
    
    // Si hay argumentos se usa SSL y puerto 443 (HTTPS)
    if (argc > 1)
    {
        port = 443; 
        client = new SSLSocket(); // Se crea un socket con soporte SSL
    }
    else
    {
        client = new Socket('s'); // Se crea un stream socket
        port = 80; // HTTP
    }

    memset(a, 0, MAXBUF); // Se limpia el buffer
    client->MakeConnection(os, port); // Conexión con el servidor en la IP y puerto especificado
    client->Write((char *)whale, strlen(whale)); // Envía la petición HTTP al servidor
    st = client->Read(a, MAXBUF); // Lee la respuesta del servidor
    printf("Bytes leídos %d\n%s\n", st, a); // Imprime el número de bytes leídos y la respuesta recibida
}