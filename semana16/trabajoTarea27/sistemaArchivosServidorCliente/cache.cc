#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <climits>
#include <regex>
#include <atomic>
#include <thread>
#include <chrono>
#include "Socket.h"
#include "Utils.h"  

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
            return -1;
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
    
    // Eliminar elemento del cache
    int remove(string name) {
        lock_guard<mutex> lock(cacheMutex);
        auto it = memory.find(name);
        if (it == memory.end()) {
            return -1; // No encontrado
        }
        
        freeSpace += it->second.content.size();
        memory.erase(it);
        return 0; // Eliminado 
    }
    
    // Obtener espacio libre
    uint16_t getFreeSpace() {
        lock_guard<mutex> lock(cacheMutex);
        return freeSpace;
    }
    
    // Listar todas las figuras en cache
    string list() {
        lock_guard<mutex> lock(cacheMutex);
        if (memory.empty()) {
            return "";
        }
        
        string result;
        for (auto& [name, entry] : memory) {
            result += name + "\n";
        }
        return result;
    }
};

// Cache tamaño 
Cache cache(65535);

// Variables para el handshake
atomic<bool> server_online(false);
string server_ip;
mutex handshake_mutex;

// Función para el handshake en segundo plano
void handshake_thread() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(10)); // Reintentar cada 10 segundos
        
        {
            lock_guard<mutex> lock(handshake_mutex);
            string hostname = "cache"; // o el hostname que use
            bool success = handshake(server_ip, hostname, 5678); // Puerto 5678 para cache-servidor
            
            server_online.store(success);
            
            if (success) {
                cout << "[CACHE] Handshake exitoso con servidor: " << server_ip << endl;
            } else {
                cout << "[CACHE] Handshake fallido con servidor" << endl;
            }
        }
    }
}

int main() {
    cout << "[CACHE] Iniciando servidor Cache..." << endl;
    
    // Iniciar handshake en segundo plano
    thread hs_thread(handshake_thread);
    hs_thread.detach();
    
    Socket server('s');
    
    // Usar puerto 8082 según el protocolo
    if (server.Bind(8082) < 0) {
        cerr << "[CACHE] Error al bindear puerto 8082" << endl;
        return 1;
    }
    server.MarkPassive(10);

    cout << "[CACHE] Servidor Cache iniciado en puerto 8082..." << endl;
    cout << "[CACHE] Tamaño máximo: 64KB" << endl;
    cout << "[CACHE] Protocolo: TCP 8082, UDP 5678" << endl;
    cout << "[CACHE] Esperando conexiones y handshake..." << endl;

    while (true) {
        VSocket *client = server.AcceptConnection();
        cout << "[CACHE] Cliente conectado" << endl;

        string request;
        if (!readVerify(*client, request)) {
            cerr << "[CACHE] Error leyendo solicitud" << endl;
            delete client;
            continue;
        }

        cout << "[CACHE] Solicitud recibida: " << request.substr(0, 100) << "..." << endl;

        string response;

        // Verificar si el servidor está online
        bool is_online = server_online.load();
        cout << "[CACHE] Estado servidor: " << (is_online ? "ONLINE" : "OFFLINE") << endl;

        // LIST /BEGIN/END/
        if (request.rfind("LIST", 0) == 0) {
            cout << "[CACHE] Procesando LIST" << endl;
            string listContent = cache.list();
            
            if (!listContent.empty()) {
                response = "/BEGIN/200/" + to_string(listContent.size()) + "/END/\n" + listContent;
                cout << "[CACHE] LIST enviado (" << listContent.size() << " bytes)" << endl;
            } else {
                response = "/BEGIN/404/END/\n";
                cout << "[CACHE] LIST vacío" << endl;
            }
        }
        // GET /BEGIN/filename/END/
        else if (request.rfind("GET", 0) == 0) {
            regex re("GET\\s/BEGIN/([\\w\\.]+)/");
            smatch match;
            
            if (regex_search(request, match, re)) {
                string filename = match[1];
                cout << "[CACHE] Buscando figura: " << filename << endl;
                
                string content = cache.get(filename);
                if (!content.empty()) {
                    response = "/BEGIN/200/" + to_string(content.size()) + "/END/\n" + content;
                    cout << "[CACHE] Figura '" << filename << "' encontrada (" << content.size() << " bytes)" << endl;
                } else {
                    response = "/BEGIN/404/END/\n";
                    cout << "[CACHE] Figura '" << filename << "' no encontrada" << endl;
                }
            } else {
                response = "/BEGIN/400/END/\n";
                cout << "[CACHE] GET mal formado" << endl;
            }
        }
        // ADD /BEGIN/filename/size/END/\ncontent
        else if (request.rfind("ADD", 0) == 0) {
            regex re("ADD\\s/BEGIN/([\\w\\.]+)/\\d+/END/\n([\\s\\S]*)");
            smatch match;
            
            if (regex_search(request, match, re)) {
                string filename = match[1];
                string content = match[2];
                
                cout << "[CACHE] Agregando figura: " << filename << " (" << content.size() << " bytes)" << endl;
                
                int result = cache.add(filename, content);
                if (result == 0) {
                    response = "/BEGIN/200/END/\n";
                    cout << "[CACHE] Figura '" << filename << "' agregada exitosamente" << endl;
                } else {
                    response = "/BEGIN/500/END/\n";
                    cout << "[CACHE] Error agregando figura '" << filename << "'" << endl;
                }
            } else {
                response = "/BEGIN/400/END/\n";
                cout << "[CACHE] ADD mal formado" << endl;
            }
        }
        // DELETE /BEGIN/filename/END/
        else if (request.rfind("DELETE", 0) == 0) {
            regex re("DELETE\\s/BEGIN/([\\w\\.]+)/");
            smatch match;
            
            if (regex_search(request, match, re)) {
                string filename = match[1];
                cout << "[CACHE] Eliminando figura: " << filename << endl;
                
                int result = cache.remove(filename);
                if (result == 0) {
                    response = "/BEGIN/200/END/\n";
                    cout << "[CACHE] Figura '" << filename << "' eliminada" << endl;
                } else {
                    response = "/BEGIN/404/END/\n";
                    cout << "[CACHE] Figura '" << filename << "' no encontrada para eliminar" << endl;
                }
            } else {
                response = "/BEGIN/400/END/\n";
                cout << "[CACHE] DELETE mal formado" << endl;
            }
        }
        // Comando desconocido notifico con mensaje
        else {
            response = "/BEGIN/400/END/\n";
            cout << "[CACHE] Comando desconocido" << endl;
        }

        // Enviar respuesta
        try {
            client->Write(response.c_str(), response.size());
            cout << "[CACHE] Respuesta enviada: " << response.substr(0, 50) << "..." << endl;
        } catch (exception& e) {
            cerr << "[CACHE] Error enviando respuesta: " << e.what() << endl;
        }

        delete client;
        cout << "[CACHE] Conexión cerrada" << endl;
    }
    
    return 0;
}