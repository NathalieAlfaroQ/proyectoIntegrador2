#ifndef SERVIDOR_H
#define SERVIDOR_H

// Bibliotecas
#include <iostream>
#include <string>
#include <map>
#include <pthread.h>
#include <thread>
#include <unistd.h>
#include <queue>
#include <fstream>

using namespace std;

// Buzón para mensajes, es decir, mensaje de solicitud
struct Mensaje
{
    string animal;
};

// Gestiona las figuras
// Las figuras están en memoria
// Comunicación por medio de una cola de mensajes
class Servidor
{
private:
    // Almacena las figuras y su contenido
    map<string, string> figuras;
    // Cola de mensajes para gestionar las solicitudes
    queue<Mensaje> messageQueue;
    // Mutex para controlar el acceso a la cola
    pthread_mutex_t queueMutex;
    // Hilo del Servidor
    pthread_t thread;
    // Estado del servidor
    bool corriendo = true;

    // Procesa las solicitudes en la cola
    static void *procesaSolicitud(void *arg);

public:
    // Constructor, también lee la figura del txt
    Servidor(const string &figura);
    // Destructor
    ~Servidor();

    // Para empezar y detener el hilo del servidor:
    void iniciar();
    void detener();

    // Obtiene el contenido de la figura solicitada
    string obtenerFigura(const string &figura);
    // Solicitud en la cola de mensajes del servidor para procesarla
    void enviarSolicitud(const Mensaje &msg);
};
#endif