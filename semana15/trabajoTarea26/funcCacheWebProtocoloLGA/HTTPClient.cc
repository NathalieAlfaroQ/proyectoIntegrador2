#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include "Socket.h"
#include "VSocket.h"
#define buffersize 5000

int main(int argc, char *argv[])
{
    const char *direccion = "127.0.0.1";
    std::string args;

    for (int i = 1; i < argc; i++)
    {
        args += std::string(argv[i]) + " ";
    }

    std::string command = "GET";
    std::string filename;
    std::string filecontent;

    if (args.find("--list") != std::string::npos)
    {
        command = "LIST";
    }
    else if (args.find("--add=") != std::string::npos)
    {
        command = "ADD";
        size_t pos = args.find("--add=");
        size_t end = args.find(" ", pos);
        filename += args.substr(pos + 6, end - (pos + 6));

        std::string path = "Figuras/" + filename;

        std::cout << "Abriendo archivo en ruta: " << path << "\n";

        std::ifstream file(path);
        if (!file.is_open())
        {
            std::cerr << "[ERROR] No se pudo abrir el archivo: " << filename << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        filecontent += buffer.str();
        file.close();
    }
    else if (args.find("--rm=") != std::string::npos)
    {
        command = "DELETE";
        size_t pos = args.find("--rm=");
        if (pos != std::string::npos)
        {
            size_t end = args.find(" ", pos);
            filename = args.substr(pos + 5, end - (pos + 5));
        }
        else
        {
            filename = "killerWhale.txt";
        }
    }
    else
    {
        size_t pos = args.find("--get=");
        if (pos != std::string::npos)
        {
            size_t end = args.find(" ", pos);
            filename = args.substr(pos + 6, end - (pos + 6));
        }
        else
        {
            filename = "killerWhale.txt";
        }
    }

    std::string request;
    if (command == "LIST")
    {
        request = "LIST /BEGIN//END/\n";
    }
    else if (command == "GET")
    {
        request = "GET /BEGIN/" + filename + "/END/\n";
    }
    else if (command == "DELETE")
    {
        request = "DELETE /BEGIN/" + filename + "/END/\n";
    }
    else if (command == "ADD")
    {
        request = "ADD /BEGIN/" + filename + "/" + std::to_string(filecontent.size()) + "/END/\n" + filecontent;
    }

    const char *solicitud = request.c_str();
    std::cout << "Dirección a usar: " << direccion << "\n";
    std::cout << "Solicitud enviada:\n"
              << solicitud << "\n";

    VSocket *s = nullptr;
    char buffer[buffersize];
    memset(buffer, 0, buffersize);

    try
    {
        s = new Socket('s');
        s->MakeConnection(direccion, 8081);
        s->Write(solicitud, strlen(solicitud));

        size_t bytes = s->Read(buffer, buffersize);
        std::string content(buffer, bytes);

        if (content.find("200") != std::string::npos)
        {
            std::cout << "[OK] Respuesta del servidor:\n"
                      << content << "\n";
        }
        else
        {
            std::cout << "[ERROR] Respuesta del servidor:\n"
                      << content << "\n";
        }
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "[ERROR] Excepción al leer/escribir en el socket: " << e.what() << "\n";
    }

    if (s)
        delete s;
    return 0;
}