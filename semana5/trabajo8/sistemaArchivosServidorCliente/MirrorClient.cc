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
#include <fstream>
#include <string>

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
   char buffer[BUFSIZE];
   s = new Socket('s', false);
   memset(buffer, 0, BUFSIZE);

   // Se conecta al servidor en esa IP y puerto
   s->MakeConnection("172.23.148.148", PORT);

   int opcion;
   std::cout << "Seleccione una opción:\n";
   std::cout << "1. Ver figura existente\n";
   std::cout << "2. Agregar nueva figura (desde archivo otra.txt)\n";
   std::cin >> opcion;
   std::cin.ignore(); // limpiar salto de línea

   if (opcion == 1) {
      std::string figura;
      std::cout << "Digite el nombre de la figura: hanukka, perro, torre: ";
      std::cin >> figura;

      std::string mensaje = "GET:" + figura;
      s->Write(mensaje.c_str());

      s->Read(buffer, BUFSIZE);
      std::cout << "Figura recibida:\n" << buffer << std::endl;
   } 
   else if (opcion == 2) {
      std::string nombre;
      std::cout << "Digite el nombre de la nueva figura: ";
      std::cin >> nombre;

      // Leer contenido del archivo otra.txt
      std::ifstream archivo("otra.txt");
      if (!archivo) {
         std::cerr << "Error: no se pudo abrir 'otra.txt'" << std::endl;
         s->Close();
         return 1;
      }

      std::string linea, contenido;
      while (std::getline(archivo, linea)) {
         contenido += linea + "\n";
      }
      archivo.close();

      std::string mensaje = "ADD:" + nombre + ":" + contenido;
      s->Write(mensaje.c_str());

      s->Read(buffer, BUFSIZE);
      std::cout << buffer << std::endl;
   }

   // Termina la conexión con el servidor
   s->Close();
   return 0;
}