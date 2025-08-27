#ifndef TENEDOR_H
#define TENEDOR_H

// Bibliotecas
#include <iostream>
#include <string>
#include <pthread.h>
#include <queue>

// Encabezados
#include "Servidor.h"

using namespace std;

/*
* La clase intermediaria entre el cliente y el servidor.
* Gestiona las solicitudes.
* Almacena la figura solicitada.
*/
class Tenedor {
private:
    // Guarda la figura enviada por el servidor
    string figuraGuardada; 
    // Cola de mensajes para gestionar las solicitudes
    queue<Mensaje> messageQueue; 
    // Mutex para controlar el acceso a la cola
    pthread_mutex_t queueMutex; 
    // Hilo del Tenedor
    pthread_t thread; 
    Servidor* servidor; 
    // Para ver si el Tenedor está en ejecución
    bool corriendo = true;

    // Procesa las solicitudes en la cola
    static void* procesaSolicitud(void* arg);

public:
    // Constructor y configura la conexión con el servidor
    Tenedor(Servidor* server);
    // Destructor
    ~Tenedor();

    // Para empezar y detener el hilo del tenedor:
    void iniciar();
    void detener();

    // Recibe la figura del servidor y la guarda
    void recibeFigura(const string& figura, const string& contenido);
    // Muestra la figura
    void mostrarFigura();
    // Le pide la figura al servidor
    void enviarSolicitud(const Mensaje& msg);
};
#endif