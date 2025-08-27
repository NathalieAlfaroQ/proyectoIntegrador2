// Encabezados
#include "Tenedor.h"

using namespace std;

/*
* Constructor del Tenedor y le pasamos un puntero de un objeto
* tipo Servidor que es server donde se inicializa el mutex
* para proteger la cola de mensajes.
*/
Tenedor::Tenedor(Servidor *server) : servidor(server)
{
    pthread_mutex_init(&queueMutex, nullptr);
}

// Destructor de la clase Tenedor y su mutex
Tenedor::~Tenedor()
{
    pthread_mutex_destroy(&queueMutex);
}

/*
* Se crea su hilo que trabajará con la función
* procesaSolicitud que procesa la solititud
* del cliente.
*/
void Tenedor::iniciar()
{
    pthread_create(&thread, nullptr, procesaSolicitud, this);
}

// Aquí se detiene la ejecución del Tenedor
void Tenedor::detener()
{
    corriendo = false;
    cout << "\n x Cliente - Tenedor x" << endl;
    pthread_join(thread, nullptr);
}

// Se procesa la solicitud del cliente al tenedor en su hilo
// Se pasa dicha solicitud a la cola de mensajes
void *Tenedor::procesaSolicitud(void *arg)
{
    Tenedor *server = static_cast<Tenedor *>(arg);

    // Mientras sel servidor esté activo
    while (server->corriendo)
    {
        // Bloqueamos la cola para evitar choques
        pthread_mutex_lock(&server->queueMutex);

        // Si hay mensaje en la cola
        if (!server->messageQueue.empty())
        {
            // Tomamos el primer mensaje
            Mensaje msg = server->messageQueue.front();
            // Lo quitamos de la cola
            server->messageQueue.pop();
            // Se puede desbloquear la cola para que pueda ver otro mensaje
            pthread_mutex_unlock(&server->queueMutex);
            // Se busca la figura solicitada y se guarda su contenido
            string contenido = server->servidor->obtenerFigura(msg.animal);
            // Verificamos si recibió la figura
            server->recibeFigura(msg.animal, contenido);
        }
        else
        {
            // Si no hay mensajes en la cola, se puede desbloquear
            // la cola para que escuche nuevas solicitudes
            pthread_mutex_unlock(&server->queueMutex);
        }

        sleep(1);
    }

    return nullptr;
}

// Para verificar si el Tenedor recibió la figura
void Tenedor::recibeFigura(const string &figura, const string &contenido)
{
    if (!contenido.empty())
    {
        figuraGuardada = contenido;
        cout << "\n Tenedor tiene la figura " << figura << endl;
    }
    else
    {
        cout << "\n Tenedor no recibió la figura." << endl;
    }
}

// Este es para desplegar la figura solicitada
void Tenedor::mostrarFigura()
{
    if (!figuraGuardada.empty())
    {
        cout << "\n Cliente tiene la figura por el Tenedor: \n"
                  << figuraGuardada << endl;
    }
    else
    {
        cout << "\n Tenedor no tiene la figura." << endl;
    }
}

// Aquí se envía la solicitud de la figura al servidor por
// parte de tenedor
void Tenedor::enviarSolicitud(const Mensaje &msg)
{
    cout << "\n Tenedor solicita la figura al Servidor." << endl;
    // Bloqueamos la cola para que no haya interferencia
    pthread_mutex_lock(&queueMutex);
    // Sacamos la solicitud
    messageQueue.push(msg);
    // Desbloqueamos la cola para que siga escuchando
    pthread_mutex_unlock(&queueMutex);
}