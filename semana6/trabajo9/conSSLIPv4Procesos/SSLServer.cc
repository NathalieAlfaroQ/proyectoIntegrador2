/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  ****** SSLSocket example, server code con procesos
  *
  * (Fedora version)
  *
 **/

#include <unistd.h>   // fork, _exit
#include <sys/wait.h> // waitpid
#include <cstdlib>    // atoi
#include <cstdio>     // printf
#include <cstring>    // strlen, strcmp
#include <cerrno>     // errno

#include "SSLSocket.h"

#define PORT 4321

void Service(SSLSocket *client);

int main(int cuantos, char **argumentos) {
   int port = PORT;

   if (cuantos > 1) {
      port = atoi(argumentos[1]);
   }

   // El tercer argumento "false" indica que es servidor
   SSLSocket* server = new SSLSocket("./ci0123.pem", "./key0123.pem", false);
   server->Bind(port);
   server->MarkPassive(10);

   for (;;) {
      SSLSocket *client = (SSLSocket *) server->AcceptConnection();

      pid_t pid = fork();
      if (pid < 0) {
         perror("fork");
         client->Close();
         delete client;
         continue;
      }

      if (pid == 0) {
         // Proceso hijo
         server->Close();   // el hijo no necesita el socket del servidor
         Service(client);
         delete client;
         _exit(0);          // salir limpio
      } else {
         // Proceso padre
         client->Close();   // el padre no atiende al cliente
         delete client;

         // Evitar zombies (recolectar procesos terminados)
         int status;
         waitpid(-1, &status, WNOHANG);
      }
   }
}

void Service(SSLSocket *client) {
   char buf[1024] = {0};
   int bytes;
   const char* ServerResponse =
      "\n<Body>\n"
      "\t<Server>os.ecci.ucr.ac.cr</Server>\n"
      "\t<dir>ci0123</dir>\n"
      "\t<Name>Proyecto Integrador Redes y sistemas Operativos</Name>\n"
      "\t<NickName>PIRO</NickName>\n"
      "\t<Description>Consolidar e integrar los conocimientos de redes y sistemas operativos</Description>\n"
      "\t<Author>profesores PIRO</Author>\n"
      "</Body>\n";

   const char *validMessage =
      "\n<Body>\n"
      "\t<UserName>piro</UserName>\n"
      "\t<Password>ci0123</Password>\n"
      "</Body>\n";

   client->ShowCerts(true);

   bytes = client->Read(buf, sizeof(buf));
   buf[bytes] = '\0';
   printf("Client msg: \"%s\"\n", buf);

   if (!strcmp(validMessage, buf)) {
      client->Write(ServerResponse, strlen(ServerResponse));
   } else {
      client->Write("Invalid Message", strlen("Invalid Message"));
   }

   client->Close();
}