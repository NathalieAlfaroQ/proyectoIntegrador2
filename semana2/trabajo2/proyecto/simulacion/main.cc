// make
// ./simulacion

// Bibliotecas
#include <fstream>

// Encabezados
#include "Servidor.h"
#include "Tenedor.h"
#include "Cliente.h"

using namespace std;

int main()
{
    /*
     * Le desplegamos una lista al cliente para que observe cuáles figuras de
     * de animales hay disponibles, el cliente digita por teclado su elección.
     */
    string figuraAnimal;
    cout << "Figuras disponibles, digite una en minúscula: \n - pinguino \n - perro \n"
         << endl;
    cin >> figuraAnimal;

    // Se crean los objetos
    Servidor servidor(figuraAnimal);
    Tenedor tenedor(&servidor);
    Cliente cliente(&tenedor);

    // Activar el servidor y el tenedor
    servidor.iniciar();
    tenedor.iniciar();

    // Se pasa la solicitud del cliente
    cliente.solicitaFigura(figuraAnimal);

    // Pausa para que los subprocesos procesen las solicitudes
    sleep(3);

    // Muestra la figura solicitada
    cliente.mostrarFigura();

    // Desactivar el servidor y el tenedor
    tenedor.detener();
    servidor.detener();

    // Fin del programa
    return 0;
}