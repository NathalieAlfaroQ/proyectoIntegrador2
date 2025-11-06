#include <iostream>
#include <string>
#include <sstream>
#include "Socket.h"
#include "FileSystem.h"

using namespace std;

// Variable global del FileSystem
FileSystem* fs = nullptr;

// Función para inicializar el FileSystem
void inicializarFileSystem() {
    if (fs == nullptr) {
        fs = new FileSystem("filesystem.bin");
        // Crear el filesystem si no existe
        ifstream test("filesystem.bin");
        if (!test.good()) {
            fs->Crear_FileSystem();
            cout << "[HTTP] FileSystem creado exitosamente" << endl;
        }
        test.close();
    }
}

// Función para enviar petición al cache
string consultarCache(const string &cmd) {
    Socket cache('s');
    if (cache.MakeConnection("127.0.0.1", 5000) < 0) {
        cerr << "[HTTP] Error conectando al cache" << endl;
        return "ERROR_CACHE";
    }

    // Enviar comando con terminación explícita
    string comando = cmd + "\n";
    cache.Write(comando.c_str());

    char buffer[4096];
    int n = cache.Read(buffer, sizeof(buffer) - 1);
    
    if (n <= 0) {
        cerr << "[HTTP] No se pudo leer respuesta del cache" << endl;
        return "MISS";
    }

    buffer[n] = '\0';
    return string(buffer, n);
}

// Función para actualizar el cache cuando se obtiene una figura del FileSystem
void actualizarCache(const string &nombre, const string &contenido) {
    Socket cache('s');
    if (cache.MakeConnection("127.0.0.1", 5000) < 0) {
        cerr << "[HTTP] Error conectando al cache para actualizar" << endl;
        return;
    }

    // Enviar comando para almacenar en cache
    string comando = "STORE " + nombre + "\n" + contenido + "\n";
    cache.Write(comando.c_str());
    
    char buffer[256];
    cache.Read(buffer, sizeof(buffer) - 1); // Leer respuesta
}

// Función para obtener listado desde FileSystem
string obtenerListadoFileSystem() {
    inicializarFileSystem();
    return fs->Listar_Figuras();
}

// Función para obtener figura desde FileSystem
string obtenerFiguraFileSystem(const string &nombre) {
    inicializarFileSystem();
    
    // Asegurarse de que el nombre tenga .txt
    string nombreBuscar = nombre;
    if (nombreBuscar.find(".txt") == string::npos) {
        nombreBuscar += ".txt";
    }
    
    string contenido = fs->Leer_Figura(nombreBuscar);
    
    // Verificar si no se encontró la figura
    if (contenido.find("no encontrada") != string::npos || 
        contenido.find("ERROR") != string::npos) {
        return "MISS";
    }
    
    return contenido;
}

// Función para agregar figura usando FileSystem
string agregarFiguraFileSystem(const string &nombre) {
    inicializarFileSystem();
    
    bool exito = fs->Agregar_Figura(nombre);
    if (exito) {
        // Invalidar el cache del listado
        Socket cache('s');
        if (cache.MakeConnection("127.0.0.1", 5000) >= 0) {
            cache.Write("INVALIDATE LISTADO\n");
            char buffer[256];
            cache.Read(buffer, sizeof(buffer) - 1);
        }
        return "OK";
    } else {
        return "ERROR";
    }
}

int main() {
    Socket server('s');
    if (server.Bind(8080) < 0) {
        cerr << "[HTTP] Error al bindear puerto 8080" << endl;
        return 1;
    }
    server.MarkPassive(10);

    cout << "[HTTP] Servidor HTTP escuchando en puerto 8080..." << endl;
    cout << "[HTTP] FileSystem: filesystem.bin" << endl;

    // Inicializar FileSystem al inicio
    inicializarFileSystem();

    while (true) {
        VSocket *client = server.AcceptConnection();
        cout << "\n[HTTP] Nueva conexión" << endl;

        char buffer[4096];
        int n = client->Read(buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            cerr << "[HTTP] Error leyendo solicitud del cliente" << endl;
            delete client;
            continue;
        }

        buffer[n] = '\0';
        string req(buffer, n);
        cout << "[HTTP RAW] " << req << endl;

        // Parse GET
        size_t start = req.find("GET ") + 4;
        size_t end = req.find(" ", start);
        if (start == string::npos || end == string::npos) {
            client->Write("HTTP/1.1 400 BAD REQUEST\r\n\r\nSolicitud mal formada");
            delete client;
            continue;
        }

        string path = req.substr(start, end - start);
        cout << "[HTTP] Path: " << path << endl;

        // /listado
        if (path == "/listado") {
            string resp = consultarCache("LISTADO");

            if (resp == "MISS") {
                // No está en cache, obtener del FileSystem
                cout << "[HTTP] Listado no en cache, consultando FileSystem..." << endl;
                resp = obtenerListadoFileSystem();
                
                if (resp.find("ERROR") != string::npos) {
                    string error = "HTTP/1.1 500 INTERNAL ERROR\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Connection: close\r\n"
                                  "\r\n"
                                  "Error al obtener listado";
                    client->Write(error.c_str());
                } else {
                    // Actualizar cache con el listado obtenido
                    actualizarCache("LISTADO", resp);
                    
                    string http = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "Connection: close\r\n"
                                 "\r\n" + resp;
                    client->Write(http.c_str());
                }
            } else if (resp == "ERROR_CACHE") {
                string error = "HTTP/1.1 500 INTERNAL ERROR\r\n"
                              "Content-Type: text/plain\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "Error del sistema";
                client->Write(error.c_str());
            } else {
                // Está en cache, usar directamente
                string http = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/plain\r\n"
                             "Connection: close\r\n"
                             "\r\n" + resp;
                client->Write(http.c_str());
            }

            delete client;
            continue;
        }

        // /figura?nombre=xxx
        if (path.rfind("/figura", 0) == 0) {
            size_t p = path.find("nombre=");
            if (p == string::npos) {
                client->Write("HTTP/1.1 400 BAD REQUEST\r\n\r\nFalta parametro nombre");
                delete client;
                continue;
            }

            string nombre = path.substr(p + 7);
            // Limpiar parámetros adicionales si los hay
            size_t end_param = nombre.find("&");
            if (end_param != string::npos) {
                nombre = nombre.substr(0, end_param);
            }

            cout << "[HTTP] Buscando figura: " << nombre << endl;
            string resp = consultarCache("FIGURA " + nombre);

            if (resp == "MISS") {
                // No está en cache, obtener del FileSystem
                cout << "[HTTP] Figura '" << nombre << "' no en cache, consultando FileSystem..." << endl;
                resp = obtenerFiguraFileSystem(nombre);

                if (resp == "MISS") {
                    string not_found = "HTTP/1.1 404 NOT FOUND\r\n"
                                      "Content-Type: text/plain\r\n"
                                      "Connection: close\r\n"
                                      "\r\n"
                                      "Figura no existe";
                    client->Write(not_found.c_str());
                } else {
                    // Actualizar cache con la figura obtenida
                    actualizarCache(nombre, resp);
                    
                    string http = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "Connection: close\r\n"
                                 "\r\n" + resp;
                    client->Write(http.c_str());
                }
            } else if (resp == "ERROR_CACHE") {
                string error = "HTTP/1.1 500 INTERNAL ERROR\r\n"
                              "Content-Type: text/plain\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "Error del sistema";
                client->Write(error.c_str());
            } else {
                // Está en cache, usar directamente
                string http = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/plain\r\n"
                             "Connection: close\r\n"
                             "\r\n" + resp;
                client->Write(http.c_str());
            }

            delete client;
            continue;
        }

        // /add?nombre=xxx
        if (path.rfind("/add", 0) == 0) {
            size_t p = path.find("nombre=");
            if (p == string::npos) {
                client->Write("HTTP/1.1 400 BAD REQUEST\r\n\r\nFalta parametro nombre");
                delete client;
                continue;
            }

            string nombre = path.substr(p + 7);
            // Limpiar parámetros adicionales si los hay
            size_t end_param = nombre.find("&");
            if (end_param != string::npos) {
                nombre = nombre.substr(0, end_param);
            }

            cout << "[HTTP] Agregando figura: " << nombre << endl;
            
            string resultado = agregarFiguraFileSystem(nombre);
            
            if (resultado == "OK") {
                string success = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Connection: close\r\n"
                                "\r\n"
                                "Figura agregada exitosamente";
                client->Write(success.c_str());
            } else {
                string error = "HTTP/1.1 500 INTERNAL ERROR\r\n"
                              "Content-Type: text/plain\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "Error al agregar figura";
                client->Write(error.c_str());
            }

            delete client;
            continue;
        }

        // Ruta desconocida
        client->Write("HTTP/1.1 404 NOT FOUND\r\n\r\nRuta no válida");
        delete client;
    }

    // Limpiar FileSystem al salir
    if (fs != nullptr) {
        delete fs;
    }
}