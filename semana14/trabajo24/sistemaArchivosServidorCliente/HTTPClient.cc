#include <iostream>
#include <cstring>
#include <string>
#include "Socket.h"

#define PORT 9090     // Ahora se conecta al Intermediario en puerto 9090
#define BUFSIZE 4096

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "[Uso]: " << argv[0] << " <list | add <figura> | <nombre_figura_sin_.txt>>\n";
        return 1;
    }

    std::string comando = argv[1];
    std::string host = "127.0.0.1";
    std::string request;

    if (comando == "list") {
        request = "GET /listado HTTP/1.1\r\n"
                  "Host: " + host + "\r\n"
                  "Connection: close\r\n"
                  "\r\n";
    }
    else if (comando == "add" && argc == 3) {
        std::string figura = argv[2];
        request = "GET /add?nombre=" + figura + " HTTP/1.1\r\n"
                  "Host: " + host + "\r\n"
                  "Connection: close\r\n"
                  "\r\n";
    }
    else {
        request = "GET /figura?nombre=" + comando + " HTTP/1.1\r\n"
                  "Host: " + host + "\r\n"
                  "Connection: close\r\n"
                  "\r\n";
    }

    try {
        Socket client('s');
        std::cout << "[CLIENTE] Conectando al Intermediario en " << host << ":" << PORT << std::endl;
        client.MakeConnection(host.c_str(), PORT);
        std::cout << "[CLIENTE] Enviando petición al Intermediario..." << std::endl;
        client.Write(request.c_str());

        char buffer[BUFSIZE];
        int bytesRead;

        std::cout << "\n[Respuesta del sistema]:\n\n";

        try {
            do {
                memset(buffer, 0, BUFSIZE);
                bytesRead = client.Read(buffer, BUFSIZE);
                if (bytesRead > 0) {
                    std::cout << std::string(buffer, bytesRead);
                }
            } while (bytesRead > 0);
        } catch (std::exception& ex) {
            std::string msg = ex.what();
            if (msg.find("conexión cerrada") == std::string::npos) {
                std::cerr << "[ERROR]: " << msg << std::endl;
            }
        }

        client.Close();

    } catch (std::exception& e) {
        std::cerr << "[ERROR]: " << e.what() << std::endl;
    }

    return 0;
}