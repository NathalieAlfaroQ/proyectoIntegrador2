#include <iostream>
#include <string>
#include <sstream>
#include "Socket.h"
#include "FileSystem.h"

using namespace std;

FileSystem* fs = nullptr;

void inicializarFileSystem() {
    if (fs == nullptr) {
        fs = new FileSystem("filesystem.bin");
        ifstream test("filesystem.bin");
        if (!test.good()) {
            fs->Crear_FileSystem();
            cout << "[HTTP] FileSystem creado exitosamente" << endl;
            
            // Agregar algunas figuras de prueba
            fs->Agregar_Figura("triangulo");
            fs->Agregar_Figura("cuadrado"); 
            fs->Agregar_Figura("circulo");
        }
        test.close();
    }
}

string consultarCache(const string &cmd) {
    Socket cache('s');
    if (cache.MakeConnection("127.0.0.1", 5000) < 0) {
        cerr << "[HTTP] Error conectando al cache en puerto 5000" << endl;
        return "ERROR_CACHE";
    }

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

void actualizarCache(const string &nombre, const string &contenido) {
    Socket cache('s');
    if (cache.MakeConnection("127.0.0.1", 5000) < 0) {
        cerr << "[HTTP] Error conectando al cache para actualizar" << endl;
        return;
    }

    string comando = "STORE " + nombre + "\n" + contenido + "\n";
    cache.Write(comando.c_str());
    char buffer[256];
    cache.Read(buffer, sizeof(buffer) - 1);
}

string obtenerListadoFileSystem() {
    inicializarFileSystem();
    string lista = fs->Listar_Figuras();
    cout << "[HTTP] Listado obtenido del FileSystem: " << lista.length() << " caracteres" << endl;
    return lista;
}

string obtenerFiguraFileSystem(const string &nombre) {
    inicializarFileSystem();
    
    string nombreBuscar = nombre;
    if (nombreBuscar.find(".txt") == string::npos) {
        nombreBuscar += ".txt";
    }
    
    cout << "[HTTP] Buscando figura en FileSystem: " << nombreBuscar << endl;
    string contenido = fs->Leer_Figura(nombreBuscar);
    
    if (contenido.find("no encontrada") != string::npos || 
        contenido.find("ERROR") != string::npos) {
        cout << "[HTTP] Figura no encontrada en FileSystem: " << nombreBuscar << endl;
        return "MISS";
    }
    
    cout << "[HTTP] Figura encontrada en FileSystem: " << nombreBuscar 
         << " (" << contenido.length() << " caracteres)" << endl;
    return contenido;
}

string agregarFiguraFileSystem(const string &nombre) {
    inicializarFileSystem();
    
    cout << "[HTTP] Intentando agregar figura: " << nombre << endl;
    bool exito = fs->Agregar_Figura(nombre);
    
    if (exito) {
        cout << "[HTTP] Figura agregada exitosamente: " << nombre << endl;
        // Invalidar cache del listado
        Socket cache('s');
        if (cache.MakeConnection("127.0.0.1", 5000) >= 0) {
            cache.Write("INVALIDATE LISTADO\n");
            char buffer[256];
            cache.Read(buffer, sizeof(buffer) - 1);
        }
        return "OK";
    }
    
    cout << "[HTTP] Error agregando figura: " << nombre << endl;
    return "ERROR";
}

string obtenerParametro(const string &path, const string &parametro) {
    size_t start = path.find(parametro + "=");
    if (start == string::npos) return "";
    
    start += parametro.length() + 1;
    size_t end = path.find("&", start);
    if (end == string::npos) {
        end = path.find(" ", start);
        if (end == string::npos) {
            end = path.find("HTTP/");
        }
    }
    if (end == string::npos) {
        end = path.length();
    }
    
    string valor = path.substr(start, end - start);
    // Decodificar URL encoding básico
    size_t pos;
    while ((pos = valor.find("%20")) != string::npos) {
        valor.replace(pos, 3, " ");
    }
    
    return valor;
}

int main() {
    Socket server('s');
    if (server.Bind(8080) < 0) {
        cerr << "[HTTP] Error al bindear puerto 8080" << endl;
        return 1;
    }
    server.MarkPassive(10);

    cout << "[HTTP] Servidor HTTP escuchando en puerto 8080..." << endl;
    cout << "[HTTP] Listo para recibir solicitudes del Intermediario..." << endl;

    inicializarFileSystem();

    while (true) {
        VSocket *client = server.AcceptConnection();
        cout << "\n[HTTP] === NUEVA CONEXION ===" << endl;

        char buffer[8192];
        int n = client->Read(buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            cerr << "[HTTP] Error leyendo solicitud" << endl;
            delete client;
            continue;
        }

        buffer[n] = '\0';
        string req(buffer, n);

        // Parsear método y path
        string metodo, path, version;
        istringstream iss(req);
        iss >> metodo >> path >> version;

        cout << "[HTTP] Solicitud: " << metodo << " " << path << " " << version << endl;

        // -------------------------------
        // /listado
        // -------------------------------
        if (path == "/listado") {
            cout << "[HTTP] Procesando listado de figuras..." << endl;
            string resp = consultarCache("LISTADO");

            if (resp == "MISS" || resp == "ERROR_CACHE") {
                cout << "[HTTP] Listado no en cache, consultando FileSystem..." << endl;
                resp = obtenerListadoFileSystem();
                if (resp.find("ERROR") != string::npos) {
                    string error = "HTTP/1.1 500 INTERNAL ERROR\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Connection: close\r\n"
                                  "\r\n"
                                  "Error al obtener listado";
                    client->Write(error.c_str());
                    cout << "[HTTP] Error obteniendo listado" << endl;
                } else {
                    if (resp != "MISS" && resp != "ERROR_CACHE") {
                        actualizarCache("LISTADO", resp);
                    }
                    string http = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "Connection: close\r\n"
                                 "\r\n" + resp;
                    client->Write(http.c_str());
                    cout << "[HTTP] Listado enviado (desde FileSystem)" << endl;
                }
            } else {
                string http = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/plain\r\n"
                             "Connection: close\r\n"
                             "\r\n" + resp;
                client->Write(http.c_str());
                cout << "[HTTP] Listado enviado (desde Cache)" << endl;
            }

            delete client;
            continue;
        }

        // -------------------------------
        // /figura?nombre=xxx
        // -------------------------------
        if (path.find("/figura") == 0) {
            string nombre = obtenerParametro(path, "nombre");

            if (nombre.empty()) {
                client->Write("HTTP/1.1 400 BAD REQUEST\r\n\r\nFalta parametro nombre");
                delete client;
                continue;
            }

            cout << "[HTTP] Buscando figura: " << nombre << endl;
            string resp = consultarCache("FIGURA " + nombre);

            if (resp == "MISS") {
                cout << "[HTTP] Figura no en cache, consultando FileSystem..." << endl;
                resp = obtenerFiguraFileSystem(nombre);
                if (resp == "MISS") {
                    string not_found = "HTTP/1.1 404 NOT FOUND\r\n"
                                      "Content-Type: text/plain\r\n"
                                      "Connection: close\r\n"
                                      "\r\n"
                                      "Figura '" + nombre + "' no existe";
                    client->Write(not_found.c_str());
                    cout << "[HTTP] Figura no encontrada: " << nombre << endl;
                } else {
                    actualizarCache(nombre, resp);
                    string http = "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: text/plain\r\n"
                                 "Connection: close\r\n"
                                 "\r\n" + resp;
                    client->Write(http.c_str());
                    cout << "[HTTP] Figura enviada (desde FileSystem)" << endl;
                }
            } else {
                string http = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/plain\r\n"
                             "Connection: close\r\n"
                             "\r\n" + resp;
                client->Write(http.c_str());
                cout << "[HTTP] Figura enviada (desde Cache)" << endl;
            }

            delete client;
            continue;
        }

        // -------------------------------
        // /add?nombre=xxx
        // -------------------------------
        if (path.find("/add") == 0) {
            string nombre = obtenerParametro(path, "nombre");

            if (nombre.empty()) {
                client->Write("HTTP/1.1 400 BAD REQUEST\r\n\r\nFalta parametro nombre");
                delete client;
                continue;
            }

            cout << "[HTTP] Agregando figura: " << nombre << endl;
            string resultado = agregarFiguraFileSystem(nombre);
            
            if (resultado == "OK") {
                string success = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Connection: close\r\n"
                                "\r\n"
                                "Figura '" + nombre + "' agregada exitosamente";
                client->Write(success.c_str());
                cout << "[HTTP] Figura agregada: " << nombre << endl;
            } else {
                string error = "HTTP/1.1 500 INTERNAL ERROR\r\n"
                              "Content-Type: text/plain\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "Error al agregar figura '" + nombre + "'";
                client->Write(error.c_str());
                cout << "[HTTP] Error agregando figura: " << nombre << endl;
            }

            delete client;
            continue;
        }

        // Ruta desconocida
        cout << "[HTTP] Ruta no encontrada: " << path << endl;
        client->Write("HTTP/1.1 404 NOT FOUND\r\n\r\nRuta no válida. Use: /listado, /figura?nombre=xxx, /add?nombre=xxx");
        delete client;
    }

    if (fs != nullptr) {
        delete fs;
    }
}