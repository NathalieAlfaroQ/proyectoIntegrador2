#include <stdio.h>
#include <string.h>
#include <iostream>
#include <thread>
#include <regex>
#include <csignal>
#include "VSocket.h"
#include "Socket.h"
#include "FileSystem.h"
#define buffersize 5000

const char* direccionCache = nullptr;
FileSystem* fs = nullptr;

void handle_sigint(int signum) {
    std::cout << "Servidor detenido con Ctrl+C, guardando datos...\n";
    delete fs;
    std::exit(signum);
}

std::string readProto(VSocket& sock) {
    std::string headers;
    char c;

    while (true) {
        ssize_t n = sock.Read(&c, 1);
        if (n <= 0) {
            throw std::runtime_error("Conexión cerrada antes de completar encabezado");
        }
        headers.push_back(c);

        if (headers.size() >= 4 &&
            headers.substr(headers.size() - 4) == "END\n") {
            break;
        }
    }

    size_t contentLength = 0;
    {
        size_t clPos = headers.find("contentLength");
        if (clPos != std::string::npos) {
            size_t endPos = headers.find("\n", clPos);
            std::string value = headers.substr(clPos + 13, endPos - (clPos + 13));
            contentLength = std::stoul(value);
        }
    }

    std::string data = headers;

    if (contentLength > 0) {
        std::string body(contentLength, '\0');
        size_t totalRead = 0;

        while (totalRead < contentLength) {
            ssize_t n = sock.Read(&body[totalRead], contentLength - totalRead);
            if (n <= 0) {
                throw std::runtime_error("Conexión cerrada antes de completar el body");
            }
            totalRead += n;
        }
        data += body;
    }
    return data;
}

void writeProto(VSocket& sock, const std::string& response) {
    size_t totalWritten = 0;
    const char* data = response.c_str();
    size_t size = response.size();

    while (totalWritten < size) {
        ssize_t n = sock.Write(data + totalWritten, size - totalWritten);
        if (n <= 0) {
            throw std::runtime_error("Fallo al enviar respuesta HTTP");
        }
        totalWritten += n;
    }
}

void hilo_atencion(VSocket* client) {
    try {
        std::string request = readProto(*client);

        std::cout << "[DEBUG] Se recibió la solicitud:\n" << request << "\n";

        std::string response = "\n";

        // se maneja solicitud LIST
        if (request.find("LIST") != std::string::npos) {
            std::cout << "[DEBUG] Comando LIST detectado\n";

            std::string listStr = fs->list();
            
            response = "\nBEGIN/200 OK/\ncontentLength" + std::to_string(listStr.size());
            response += "\n/END\n" + listStr;

            std::cout << "[DEBUG] Respuesta LIST preparada:\n" << response << "\n";

        }
        else if (request.find("DELETE") != std::string::npos) {
            std::cout << "[DEBUG] Comando DEL detectado\n";

            std::regex re("BEGIN/([\\w\\.]+)/");
            std::smatch match;
            if (std::regex_search(request, match, re)) {
                std::string filename = match[1];
                std::cout << "[DEBUG] Nombre de archivo a eliminar: " << filename << "\n";

                int i = fs->rm(filename);
                if (!i) {
                    response = "\nBEGIN/200 OK/Delete figure: "+ filename +"\n/END\n";
                    std::cout << "[DEBUG] Archivo encontrado, respuesta preparada\n";
                } else {
                    response = "\nBEGIN/404 NOT FOUND/\nEND\n";
                    std::cout << "[DEBUG] Archivo no encontrado\n";
                }
            }
        }
        // se maneja solicitud GET
        else if (request.find("GET") != std::string::npos) {
            std::cout << "[DEBUG] Comando GET detectado\n";

            std::regex re("BEGIN/([\\w\\.]+)/");
            std::smatch match;
            if (std::regex_search(request, match, re)) {
                std::string filename = match[1];
                std::string content;
                std::cout << "[DEBUG] Nombre de archivo a obtener: " << filename << "\n";

                try {
                    Socket* s = new Socket('s');
                    s->MakeConnection(direccionCache, 8082);
                    writeProto(*s, request);
                    
                    std::string answerCache = readProto(*s);
                    
                    
                    delete s;

                    if(answerCache.find("404 NOT FOUND")!= std::string::npos) {
                        content = fs->get(filename);
                        if (!content.empty()) {
                            response = "\nBEGIN/200 OK/\ncontentLength" + std::to_string(content.size()) + "/\nEND\n"+ content;

                            answerCache = "ADD \nBEGIN/" + filename + "/\ncontentLength" + std::to_string(content.size()) + "/\nEND\n" + content;
                            Socket* s = new Socket('s');
                            s->MakeConnection(direccionCache, 8082);
                            writeProto(*s, answerCache);

                            std::cout << "[DEBUG] Archivo encontrado, respuesta preparada\n";
                            delete s;
                        } else {
                            response = "\nBEGIN/404 NOT FOUND/\nEND\n";
                            std::cout << "[DEBUG] Archivo no encontrado\n";
                        }
                    } else {
                        response = answerCache;
                    }
                } catch(std::exception& e) {
                    content = fs->get(filename);
                    if (!content.empty()) {
                        response = "\nBEGIN/200 OK/\ncontentLength" + std::to_string(content.size()) + "/\nEND\n"+ content;
                        std::cout << "[DEBUG] Archivo encontrado, respuesta preparada\n";
                    } else {
                        response = "\nBEGIN/404 NOT FOUND/\nEND\n";
                        std::cout << "[DEBUG] Archivo no encontrado\n";
                    }
                }             
  
            }
        }
        // se maneja solicitud ADD
        else if (request.find("ADD") != std::string::npos) {
            std::cout << "[DEBUG] Comando ADD detectado\n";

            std::regex re("BEGIN/([\\w\\.]+)/\\ncontentLength\\d+/\\nEND\\n([\\s\\S]*)");
            std::smatch match;
            if (std::regex_search(request, match, re)) {
                std::string filename = match[1];
                std::string content = match[2];

                std::cout << "[DEBUG] Nombre de archivo a agregar: " << filename << "\n";
                std::cout << "[DEBUG] Contenido del archivo:\n" << content << "\n";

                fs->add(filename, content);
                response = "\nBEGIN/200 OK/\n/END\n";
                std::cout << "[DEBUG] Archivo agregado correctamente\n";
            } else {
                response = "\nBEGIN/400 BAD REQUEST/\nEND\n";
                std::cout << "[DEBUG] Error al parsear solicitud ADD\n";
            }
        } 
        else {
            response = "\nBEGIN/400 BAD REQUEST/\nEND\n";
            std::cout << "[DEBUG] Comando no reconocido\n";
        }

        writeProto(*client,response);
        std::cout << "[DEBUG] Respuesta enviada al cliente\n";

    } catch (const std::runtime_error &e) {
        std::cerr << "[ERROR] Socket exception: " << e.what() << "\n";
    }

    delete client;  // Cerramos la conexión

    std::cout << "[DEBUG] Conexión con cliente cerrada\n";
}

int main(int argc, char** argv) {
    fs = new FileSystem();
    std::signal(SIGINT, handle_sigint);

    std::string direccion = argc > 1? argv[1] : "127.0.0.1";
    direccionCache = direccion.c_str();
    
    int port = 8080;

    Socket server('s');          
    server.Bind(port);
    server.MarkPassive(5);

    std::cout << "Servidor activo en puerto " << port << "\n";

    while (true) {
        VSocket* client = server.AcceptConnection();
        std::thread worker(hilo_atencion, client); 
        worker.detach();  
    }
    
    delete fs;

    return 0;
}