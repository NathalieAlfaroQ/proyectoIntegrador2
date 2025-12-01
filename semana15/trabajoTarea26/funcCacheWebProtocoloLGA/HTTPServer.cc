#include <stdio.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <regex>
#include <csignal>
#include <mutex>
#include <sys/stat.h>
#include <fstream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <atomic>
#include "VSocket.h"
#include "Socket.h"
#include "FileSystem.h"
#include "Utils.h"

FileSystem* fileSystem = nullptr;

std::string direccionConexion;
std::string hostname;

std::mutex handshake_mutex;
std::atomic<bool> cache_is_recovering(false);
std::atomic<bool> cache_is_online(true);

std::atomic<bool> end(false);

void handle_sigint(int) {
    end.store(true);
}

bool crearDirectorio(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return mkdir(path.c_str(), 0755) == 0;
    }
    return S_ISDIR(st.st_mode);
}

void hilo_atencion(VSocket* client) {
    try {
        bool error = false;
        std::string request;
        readVerify(*client, request);

        std::cout << "[DEBUG] Solicitud recibida\n";

        std::string response;

        bool isLIST   = request.rfind("LIST",   0) == 0;
        bool isGET    = request.rfind("GET",    0) == 0;
        bool isADD    = request.rfind("ADD",    0) == 0;

        if (isLIST) {
            std::string listStr = fileSystem->Listar_Figuras();
            response = "/BEGIN/200/" + std::to_string(listStr.size()) +
                       "/END/\n" + listStr;
            std::cout << "[DEBUG] Respuesta LIST enviada\n";
        }
        else if (isGET) {
            std::regex re("GET\\s/BEGIN/([\\w\\.]+)/");
            std::smatch match;

            if (std::regex_search(request, match, re)) {
                std::string filename = match[1];
                std::string content;

                try {
                    if (!cache_is_online.load()) {
                        content = fileSystem->Leer_Figura(filename);

                        if (!content.empty() && content.find("[INFO] Figura '") == std::string::npos) {
                            response = "/BEGIN/200/" + std::to_string(content.size()) + "/END/\n" + content;
                        } else {
                            response = "/BEGIN/404/END/\n";
                        }
                    } else {
                        if (direccionConexion.empty())
                            throw std::runtime_error("cache no disponible");

                        Socket cache('s');
                        cache.MakeConnection(direccionConexion.c_str(), 8082);
                        cache.Write(request.c_str(), request.size());

                        std::string answerCache;
                        readVerify(cache, answerCache);

                        if (answerCache.find("404") != std::string::npos) {
                            content = fileSystem->Leer_Figura(filename);
                            if (!content.empty() && content.find("[INFO] Figura '") == std::string::npos) {
                                response = "/BEGIN/200/" + std::to_string(content.size()) + "/END/\n" + content;

                                // Actualizar cache
                                std::string addReq = "ADD /BEGIN/" + filename + "/" +
                                                    std::to_string(content.size()) + "/END/\n" + content;
                                Socket s2('s');
                                s2.MakeConnection(direccionConexion.c_str(), 8082);
                                s2.Write(addReq.c_str(), addReq.size());
                            } else {
                                response = "/BEGIN/404/END/\n";
                            }
                        } else {
                            response = answerCache;
                        }
                    }
                } catch (std::exception& e) {
                    content = fileSystem->Leer_Figura(filename);

                    if (!content.empty() && content.find("[INFO] Figura '") == std::string::npos) {
                        response = "/BEGIN/200/" + std::to_string(content.size()) + "/END/\n" + content;
                    } else {
                        response = "/BEGIN/404/END/\n";
                    }
                    error = true;
                }
            } else {
                response = "/BEGIN/400/END/\n";
            }
        }
        else if (isADD) {
            std::regex re("ADD\\s/BEGIN/([\\w\\.]+)/\\d+/END/\n([\\s\\S]*)");
            std::smatch match;

            if (std::regex_search(request, match, re)) {
                std::string filename = match[1].str();
                std::string content = match[2].str();
                
                // Asegurar que el nombre tenga .txt
                if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") {
                    filename += ".txt";
                }
                
                std::cout << "[DEBUG] Intentando agregar figura: " << filename << "\n";
                
                // Crear el archivo directamente en Figuras/
                std::string filePath = "Figuras/" + filename;
                std::ofstream archivo(filePath);
                
                if (!archivo.is_open()) {
                    std::cerr << "[ERROR] No se pudo crear archivo: " << filePath << "\n";
                    response = "/BEGIN/500/END/\n";
                } else {
                    archivo << content;
                    archivo.close();
                    
                    std::cout << "[DEBUG] Archivo creado en: " << filePath << "\n";
                    
                    // Ahora usar Agregar_Figura con solo el nombre del archivo
                    bool success = fileSystem->Agregar_Figura(filename);
                    
                    if (success) {
                        response = "/BEGIN/200/END/\n";
                        std::cout << "[DEBUG] Figura agregada exitosamente\n";
                    } else {
                        response = "/BEGIN/500/END/\n";
                        std::cerr << "[ERROR] Falló Agregar_Figura()\n";
                    }
                }
            } else {
                response = "/BEGIN/400/END/\n";
            }
        }
        else {
            response = "/BEGIN/400/END/\n";
        }
        
        client->Write(response.c_str(), response.size());

        if (error) {
            bool expected = false;
            if (!cache_is_recovering.compare_exchange_strong(expected, true)) {
                return;
            }

            {
                std::lock_guard<std::mutex> guard(handshake_mutex);
                bool ok = handshake(direccionConexion, hostname, 5687);
                cache_is_online.store(ok);
            }

            cache_is_recovering.store(false);
        }

    } catch (const std::exception& e) {
        std::cout << "[ERROR] Socket exception: " << e.what() << "\n";
    }

    delete client;
    std::cout << "[DEBUG] Conexión cerrada\n";
}

int main(int argc, char** argv) {   
    
    std::signal(SIGINT, handle_sigint);
    std::signal(SIGPIPE, SIG_IGN);

    hostname = argc < 2 ? "figure5" : argv[1];
    handshake(direccionConexion, hostname, 5678);

    // Inicializar FileSystem
    fileSystem = new FileSystem("filesystem.bin");
    
    // Verificar si el filesystem existe
    std::ifstream test("filesystem.bin");
    if (!test.good()) {
        fileSystem->Crear_FileSystem();
        std::cout << "[INFO] Filesystem creado\n";
    } else {
        std::cout << "[INFO] Filesystem ya existe\n";
    }
    test.close();
    
    // Verificar directorio Figuras
    crearDirectorio("Figuras");
 
    int port = 8081;
    Socket server('s');          
    server.Bind(port);
    server.MarkPassive(5);

    std::cout << "Servidor HTTP activo en puerto " << port << "\n";

    int serverFd = server.returnFd();

    while (true) {
        if (end.load()) break;

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(serverFd, &rfds);

        timeval tv{};
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int sel = select(serverFd + 1, &rfds, nullptr, nullptr, &tv);

        if (sel <= 0) {
            continue;
        }

        if (FD_ISSET(serverFd, &rfds)) {
            VSocket* client = server.AcceptConnection();
            if (client) {
                std::thread worker(hilo_atencion, client);
                worker.detach();
            }
        }
    }

    std::cout << "Servidor detenido\n";
    delete fileSystem;

    return 0;
}