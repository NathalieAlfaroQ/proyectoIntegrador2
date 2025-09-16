/*
 * Socket client/server
 * Versión Fedora
 *
 * Este programa crea un servidor que escucha conexiones de clientes.
 * Cuando un cliente se conecta, el servidor crea un proceso hijo para
 * atender la conexión, recibe un mensaje y se lo devuelve.
 */

// Bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h> // memset
#include <unistd.h>
#include <iostream>

using namespace std;

// Encabezados
#include "Socket.h"

#define PORT 1234
#define BUFSIZE 250

int main(int argc, char **argv)
{
   VSocket *s1, *s2;
   int childpid;
   char a[BUFSIZE];

   // Crea un socket de tipo stream (TCP) IPv4
   s1 = new Socket('s', false);
   // Asocia el socket al puerto especificado
   s1->Bind(PORT);
   // Marca el socket como pasivo, con una cola de espera de 5 conexiones
   s1->MarkPassive(5);

   for (;;)
   {
      // Espera y acepta una nueva conexión
      s2 = s1->AcceptConnection();
      // Crea un proceso hijo para atender la conexión
      childpid = fork();

      if (childpid < 0)
      {
         perror("server: fork error");
      }
      else
      {
         // Código que ejecuta el proceso hijo
         if (0 == childpid)
         {
            // El hijo cierra el socket original
            s1->Close();
            memset(a, 0, BUFSIZE);
            // Lee datos enviados por el cliente
            s2->Read(a, BUFSIZE);
            cout << "Servidor recibe: " << a << endl;
            // Fuerza la impresion
            cout.flush();
            // Envía los mismos datos de vuelta al cliente
            s2->Write(a);
            // Termina el proceso hijo
            exit(0);
         }
      }
      // El padre cierra la conexión ya atendida
      s2->Close();
   }
}