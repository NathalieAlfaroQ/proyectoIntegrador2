#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <climits>
#include "Socket.h"


using namespace std;

// Estructura para almacenar figuras con frecuencia de uso
struct CacheEntry {
    string content;
    uint32_t frequency;
};

class Cache {
private:
    unordered_map<string, CacheEntry> memory;
    uint16_t size;
    uint16_t freeSpace;
    mutex cacheMutex;
    
public:
    Cache(uint16_t sizeC) : size(sizeC), freeSpace(sizeC) {}
    
    // Agregar elemento al cache con política LFU
    int add(string name, string content) {
        lock_guard<mutex> lock(cacheMutex);
        
        if (content.size() > size) {
            memory.clear();
            freeSpace = size;
            return 0;
        }
        
        // Liberar espacio si es necesario (LFU)
        while (freeSpace < content.size() && !memory.empty()) {
            string toRemove;
            uint32_t minFrequency = UINT_MAX;
            
            for (auto& [key, value] : memory) {
                if (value.frequency < minFrequency) {
                    toRemove = key;
                    minFrequency = value.frequency;
                }
            }
            
            freeSpace += memory[toRemove].content.size();
            memory.erase(toRemove);
        }
        
        memory[name] = CacheEntry{content, 1};
        freeSpace -= content.size();
        return 0;
    }
    
    // Obtener elemento del cache
    string get(string name) {
        lock_guard<mutex> lock(cacheMutex);
        auto it = memory.find(name);
        if (it == memory.end()) return "";
        
        memory[name].frequency++;
        return memory[name].content;
    }
    
    // Verificar si existe un elemento
    bool exists(string name) {
        lock_guard<mutex> lock(cacheMutex);
        return memory.find(name) != memory.end();
    }
    
    // Invalidar elemento del cache
    void invalidate(string name) {
        lock_guard<mutex> lock(cacheMutex);
        auto it = memory.find(name);
        if (it != memory.end()) {
            freeSpace += it->second.content.size();
            memory.erase(it);
        }
    }
    
    // Obtener espacio libre
    uint16_t getFreeSpace() {
        return freeSpace;
    }
};

// Cache global con tamaño de 64KB
Cache cache(65536);

int main() {
    Socket server('s');
    
    // Usar puerto 8082 según el protocolo
    if (server.Bind(8082) < 0) {
        cerr << "[CACHE] Error al bindear puerto 8082" << endl;
        return 1;
    }
    server.MarkPassive(10);

    cout << "[CACHE] Servidor Cache iniciado en puerto 8082..." << endl;
    cout << "[CACHE] Tamaño máximo: 64KB" << endl;
    cout << "[CACHE] Esperando conexiones..." << endl;

    while (true) {
        VSocket *client = server.AcceptConnection();
        cout << "[CACHE] Cliente conectado" << endl;

        char buffer[8192];
        int n = client->Read(buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            delete client;
            continue;
        }

        buffer[n] = '\0';
        string req(buffer);
        req.erase(req.find_last_not_of("\r\n") + 1);

        cout << "[CACHE] Solicitud: " << req << endl;

        // -------------------------------
        // LISTADO
        // -------------------------------
        if (req == "LISTADO") {
            if (cache.exists("LISTADO")) {
                cout << "[CACHE] LISTADO encontrado en cache" << endl;
                string contenido = cache.get("LISTADO");
                client->Write(contenido.c_str());
            } else {
                cout << "[CACHE] LISTADO no encontrado en cache" << endl;
                client->Write("MISS");
            }
            delete client;
            continue;
        }

        // -------------------------------
        // FIGURA nombre
        // -------------------------------
        if (req.rfind("FIGURA ", 0) == 0) {
            string nombre = req.substr(7);

            if (cache.exists(nombre)) {
                cout << "[CACHE] Figura '" << nombre << "' encontrada en cache" << endl;
                string contenido = cache.get(nombre);
                client->Write(contenido.c_str());
            } else {
                cout << "[CACHE] Figura '" << nombre << "' no encontrada en cache" << endl;
                client->Write("MISS");
            }
            delete client;
            continue;
        }

        // -------------------------------
        // STORE nombre\ncontenido
        // -------------------------------
        if (req.rfind("STORE ", 0) == 0) {
            size_t nl_pos = req.find('\n');
            if (nl_pos != string::npos) {
                string nombre = req.substr(6, nl_pos - 6);
                string contenido = req.substr(nl_pos + 1);
                
                int resultado = cache.add(nombre, contenido);
                if (resultado == 0) {
                    cout << "[CACHE] Almacenado en cache: " << nombre 
                         << " (" << contenido.length() << " bytes)" 
                         << " - Espacio libre: " << cache.getFreeSpace() << " bytes" << endl;
                    client->Write("OK");
                } else {
                    client->Write("ERROR");
                }
            } else {
                client->Write("ERROR");
            }
            delete client;
            continue;
        }

        // -------------------------------
        // INVALIDATE nombre
        // -------------------------------
        if (req.rfind("INVALIDATE ", 0) == 0) {
            string nombre = req.substr(11);
            
            cache.invalidate(nombre);
            cout << "[CACHE] Invalidado: " << nombre 
                 << " - Espacio libre: " << cache.getFreeSpace() << " bytes" << endl;
            
            client->Write("OK");
            delete client;
            continue;
        }

        // -------------------------------
        // STATS (nuevo comando para debugging)
        // -------------------------------
        if (req == "STATS") {
            string stats = "Espacio libre: " + to_string(cache.getFreeSpace()) + " bytes\n";
            client->Write(stats.c_str());
            delete client;
            continue;
        }

        cout << "[CACHE] Comando desconocido: " << req << endl;
        client->Write("ERROR");
        delete client;
    }
}