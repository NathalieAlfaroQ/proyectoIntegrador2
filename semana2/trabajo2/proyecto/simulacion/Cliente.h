#ifndef CLIENTE_H
#define CLIENTE_H

// Bibliotecas
#include <iostream>
#include <string>

// Encabezado
#include "Tenedor.h"

using namespace std;

// Clase Cliente que es el que pide las figuras
class Cliente
{
private:
    // Puntero a Tenedor que gestiona las solicitudes
    Tenedor *tenedor;

public:
    // Constructor
    Cliente(Tenedor *server);

    // Solicita figura al Tenedor
    void solicitaFigura(const string &animal);
    // Muestra la figura que tiene el Tenedor
    void mostrarFigura();
};
#endif