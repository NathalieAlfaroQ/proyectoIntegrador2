// Compilar: g++ -o simulacion  simulacion.cpp
// Ejecutar: ./simulacion
// Tengo errores de sincronizacion

#include <iostream> // cout
#include <fstream>  // Lee archivos de texto
#include <unistd.h> // Pipes
#include <cstring>
#include <sys/types.h> // Para tipos de datos de procesos
#include <sys/wait.h>  // Esperar a que los procesos hijos terminen

using namespace std;

// Lee una figura de caracteres de un archivo de texto
string leerFigura(const string &nombreArchivo)
{
    // Abre el archivo para leerlo
    ifstream archivo(nombreArchivo);

    if (!archivo)
    {
        return "La figura no esta";
    }

    string linea, contenido;
    // Lee cada línea del archivo
    while (getline(archivo, linea))
    {
        // y lo guarda en un string
        contenido += linea + "\n";
    }

    // Cierra el archivo
    archivo.close();
    return contenido;
}

// Simulacion del tenedor con Pipe que se como el servidor
void tenedor(int pipe_lectura, int pipe_escritura)
{
    // No vamos a escribir, porque hay que leer la solicitud del cliente
    close(pipe_escritura);

    // Aqui estara el mensaje del cliente
    char buffer[100];
    // y aqui se lee el mensaje
    read(pipe_lectura, buffer, sizeof(buffer));

    // Guardamos el mensaje como string
    string request(buffer); 

    // Aqui guardaremos la respuesta del cliente, la figura que pide
    string response;       
    if (request == "pinguino")
    {
        response = leerFigura("pinguino.txt");
    }
    else if (request == "oso")
    {
        response = leerFigura("oso.txt");
    }
    else
    {
        response = "La figura no existe";
    }

    // Se envia la figura que solicitaron
    cout << "TENEDOR -> Envia la figura\n" << request << endl;
    write(pipe_escritura, response.c_str(), response.size() + 1);

    // Ya no hay que leer
    close(pipe_lectura);
    close(pipe_escritura);
}

// Simulacion del cliente con Pipe, es quien solicita la figura
void cliente(int pipe_lectura, int pipe_escritura)
{
    // No vamos a leer, porque el cliente debe escribir
    close(pipe_lectura);

    // El cliente solicita una figura al tenedor
    string solicitud = "pinguino";
    cout << "CLIENTE -> Solicita " << solicitud << "\n";
    write(pipe_escritura, solicitud.c_str(), solicitud.size() + 1);

    // Para guardar la respuesta del tenedor
    char buffer[300];                  
    // Para leer la respuesta del tenedor           
    read(pipe_lectura, buffer, sizeof(buffer));
    // Para imprimir la figura
    cout << "CLIENTE -> Recibe figura \n" << buffer << endl;

    // No vamos a escribir mas
    close(pipe_escritura);
}

int main()
{
    // Creamos el Pipe
    int pipe_fd_cliente[2];
    int pipe_fd_tenedor[2];

    pipe(pipe_fd_cliente); 
    pipe(pipe_fd_tenedor);

    // Con Proceso, por lo cual se crea el proceso
    pid_t pid = fork();

    // Verificamos ID del proceso
    if (pid == 0)
    {
        // Proceso hijo llama al TENEDOR
        tenedor(pipe_fd_cliente[0], pipe_fd_tenedor[1]);
    }
    else
    {
        // Proceso padre llama al CLIENTE
        cliente(pipe_fd_tenedor[0], pipe_fd_cliente[1]);       
        // Esperar al hijo
        wait(NULL);                      
    }

    return 0;
}