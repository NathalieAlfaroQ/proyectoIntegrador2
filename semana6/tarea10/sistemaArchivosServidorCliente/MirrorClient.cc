#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>

#include "Socket.h"
#include "lector_figura.h"

#define PORT 1234
#define BUFSIZE 2048

int main(int argc, char **argv)
{
   VSocket *s;
   char buffer[BUFSIZE];
   s = new Socket('s', false);
   memset(buffer, 0, BUFSIZE);

   // Conexión con el servidor
   s->MakeConnection("172.23.148.148", PORT);

   int opcion;
   do {
      std::cout << "\nSeleccione una opción:\n";
      std::cout << "1. Ver figura existente\n";
      std::cout << "2. Agregar nueva figura (desde archivo otra.txt)\n";
      std::cout << "3. Salir\n";
      std::cin >> opcion;
      std::cin.ignore();

      if (opcion == 1) {
         s->Write("LIST");
         memset(buffer, 0, BUFSIZE);
         s->Read(buffer, BUFSIZE);
         std::cout << "\n" << buffer << std::endl;

         std::string figura;
         std::cout << "Digite el nombre de la figura: ";
         std::cin >> figura;

         std::string mensaje = "GET:" + figura;
         s->Write(mensaje.c_str());

         memset(buffer, 0, BUFSIZE);
         s->Read(buffer, BUFSIZE);
         std::cout << "Figura recibida:\n" << buffer << std::endl;
      } 
      else if (opcion == 2) {
         std::string nombre;
         std::cout << "Digite el nombre de la nueva figura: ";
         std::cin >> nombre;

         std::ifstream archivo("otra.txt");
         if (!archivo) {
            std::cerr << "Error: no se pudo abrir 'otra.txt'" << std::endl;
            continue;
         }

         std::string linea, contenido;
         while (std::getline(archivo, linea)) {
            contenido += linea + "\n";
         }
         archivo.close();

         std::string mensaje = "ADD:" + nombre + ":" + contenido;
         s->Write(mensaje.c_str());

         memset(buffer, 0, BUFSIZE);
         s->Read(buffer, BUFSIZE);
         std::cout << buffer << std::endl;

         std::cout << "\nLista actualizada de figuras:\n";
         s->Write("LIST");  
         memset(buffer, 0, BUFSIZE);
         s->Read(buffer, BUFSIZE);
         std::cout << buffer << std::endl;
      }
   } while (opcion != 3);

   s->Close();
   return 0;
}
