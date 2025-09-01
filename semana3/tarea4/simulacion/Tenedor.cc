// Encabezados
#include "Tenedor.h"

using namespace std;

/**
 *  Clase constructor
 */
Tenedor::Tenedor(Servidor *server) : servidor(server)
{
    pthread_mutex_init(&queueMutex, nullptr);
}

/**
 *  Clase destructor
 */
Tenedor::~Tenedor()
{
    pthread_mutex_destroy(&queueMutex);
}

/**
 * Inicia el hilo.
 */
void Tenedor::iniciar()
{
    pthread_create(&thread, nullptr, procesaSolicitud, this);
    cout << "Tenedor encendido, IP 192.168.1.10, Puerto 1234." << endl;
}

/**
 * Detiene.
 * Espera a que el hilo del servidor finalice.
 */
void Tenedor::detener()
{
    cout << "\n Termina conexión Cliente-Tenedor." << endl;
    corriendo = false;
    pthread_join(thread, nullptr);
}

/**
 *  Procesa las solicitudes entrantes en la cola de mensajes del servidor.
 *  Este método se ejecuta dentro del hilo del servidor y maneja las solicitudes de forma asincrónica.
 */
void *Tenedor::procesaSolicitud(void *arg)
{
    Tenedor *server = static_cast<Tenedor *>(arg);

    while (server->corriendo)
    {
        pthread_mutex_lock(&server->queueMutex);

        if (!server->messageQueue.empty())
        {
            Mensaje msg = server->messageQueue.front();
            server->messageQueue.pop();
            pthread_mutex_unlock(&server->queueMutex);
            string contenido = server->servidor->obtenerFigura(msg.animal);
            server->recibeFigura(msg.animal, contenido);
        }
        else
        {
            pthread_mutex_unlock(&server->queueMutex);
        }

        sleep(1);
    }

    return nullptr;
}

/**
 *  Recibe una figura de la cola.
 */
void Tenedor::recibeFigura(const string &figura, const string &contenido)
{
    if (!contenido.empty())
    {
        figuraGuardada = contenido;
        cout << "\n Tenedor tiene la figura de " << figura << endl;
    }
    else
    {
        cout << "\n Tenedor no recibió la figura." << endl;
    }
}

/**
 * Muestra la figura solicitada.
 */
void Tenedor::mostrarFigura()
{
    if (!figuraGuardada.empty())
    {
        cout << "\n Cliente tiene la figura por el Tenedor: \n"
             << figuraGuardada << endl;
    }
}

/**
 * Envia la solicitud al servidor para que la procese.
 */
void Tenedor::enviarSolicitud(const Mensaje &msg)
{
    if (corriendo == false)
    {
        cout << "No se puede establecer conexión con el Tenedor." << endl;
        return;
    }

    cout << "\n Tenedor solicita la figura al Servidor." << endl;
    // Bloqueamos la cola para que no haya interferencia
    pthread_mutex_lock(&queueMutex);
    // Sacamos la solicitud
    messageQueue.push(msg);
    // Desbloqueamos la cola para que siga escuchando
    pthread_mutex_unlock(&queueMutex);
}