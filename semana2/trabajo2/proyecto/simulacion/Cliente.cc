// Encabezado
#include "Cliente.h"

using namespace std;

// Constructor
// Puntero a Tenedor que gestiona las solicitudes
Cliente::Cliente(Tenedor *server) : tenedor(server) {}

// Envía la solicitud al Tenedor para tener la figura
void Cliente::solicitaFigura(const string &animal)
{
    cout << "\n Enviando solicitud del Cliente al Tenedor." << endl;
    Mensaje msg;
    msg.animal = animal;
    tenedor->enviarSolicitud(msg);
}

// Muestra la figura guardada
void Cliente::mostrarFigura()
{
    tenedor->mostrarFigura();
}