#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include "Socket.h"

using namespace std;

map<string, string> cacheMem;

int main() {
    Socket server('s');
    if (server.Bind(5000) < 0) {
        cerr << "[CACHE] Error al bindear puerto 5000" << endl;
        return 1;
    }
    server.MarkPassive(5);

    cout << "[CACHE] Servidor Cache iniciado en puerto 5000..." << endl;

    while (true) {
        VSocket *client = server.AcceptConnection();
        cout << "[CACHE] Cliente conectado." << endl;

        char buffer[8192]; // Buffer más grande para almacenar contenido
        int n = client->Read(buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            cerr << "[CACHE] Error leyendo solicitud del cliente" << endl;
            delete client;
            continue;
        }

        buffer[n] = '\0';
        string req(buffer);
        req.erase(req.find_last_not_of("\r\n") + 1);

        cout << "[CACHE] Solicitud: " << req.substr(0, 50) << "..." << endl;

        // LISTADO
        if (req == "LISTADO") {
            if (cacheMem.count("LISTADO")) {
                cout << "[CACHE] LISTADO encontrado en cache" << endl;
                client->Write(cacheMem["LISTADO"].c_str());
            } else {
                cout << "[CACHE] LISTADO no encontrado en cache" << endl;
                client->Write("MISS");
            }
            delete client;
            continue;
        }

        // FIGURA nombre
        if (req.rfind("FIGURA ", 0) == 0) {
            string nombre = req.substr(7);

            if (cacheMem.count(nombre)) {
                cout << "[CACHE] Figura '" << nombre << "' encontrada en cache" << endl;
                client->Write(cacheMem[nombre].c_str());
            } else {
                cout << "[CACHE] Figura '" << nombre << "' no encontrada en cache" << endl;
                client->Write("MISS");
            }
            delete client;
            continue;
        }

        // STORE nombre\ncontenido
        if (req.rfind("STORE ", 0) == 0) {
            size_t nl_pos = req.find('\n');
            if (nl_pos != string::npos) {
                string nombre = req.substr(6, nl_pos - 6);
                string contenido = req.substr(nl_pos + 1);
                
                cacheMem[nombre] = contenido;
                cout << "[CACHE] Almacenado en cache: " << nombre << " (" 
                     << contenido.length() << " bytes)" << endl;
                
                client->Write("OK");
            } else {
                client->Write("ERROR");
            }
            delete client;
            continue;
        }

        // INVALIDATE nombre
        if (req.rfind("INVALIDATE ", 0) == 0) {
            string nombre = req.substr(11);
            
            if (cacheMem.count(nombre)) {
                cacheMem.erase(nombre);
                cout << "[CACHE] Invalidado: " << nombre << endl;
            }
            
            client->Write("OK");
            delete client;
            continue;
        }

        cout << "[CACHE] Comando desconocido: " << req << endl;
        client->Write("MISS");
        delete client;
    }
}