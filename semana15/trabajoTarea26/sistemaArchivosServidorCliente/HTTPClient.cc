#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <regex>
#include "Socket.h"
#include "VSocket.h"
#define buffersize 50000

//  mostrar figuras del protocolo
void displayProtocolFiguras(const std::string& content) {
    // Protocolo: extraer lista después de /END/\n
    std::regex list_re("/BEGIN/200/\\d+/END/\n([\\s\\S]*)");
    std::smatch match;
    
    if (std::regex_search(content, match, list_re)) {
        std::cout << "\n=== FIGURAS DISPONIBLES ===\n";
        std::string list_str = match[1];
        std::istringstream stream(list_str);
        std::string figura;
        
        while (std::getline(stream, figura)) {
            if (!figura.empty() && figura != "\n" && figura != "\r") {
                if (!figura.empty() && figura.back() == '\r') {
                    figura.pop_back();
                }
                if (!figura.empty()) {
                    std::cout << "• " << figura << "\n";
                }
            }
        }
        std::cout << "===========================\n";
    } else {
        std::cout << "[OK] Respuesta:\n" << content << "\n";
    }
}

//  mostrar contenido de figura del protocolo
void displayProtocolFiguraContent(const std::string& content) {
    // Protocolo: extraer contenido después de /END/\n
    std::regex content_re("/BEGIN/200/\\d+/END/\n([\\s\\S]*)");
    std::smatch match;
    
    if (std::regex_search(content, match, content_re)) {
        std::cout << "\n" << match[1] << "\n";
    } else {
        std::cout << content << "\n";
    }
}

//  mostrar respuesta HTTP 
void displayHTTPResponse(const std::string& content) {
    // Separar headers del body
    size_t header_end = content.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        std::string body = content.substr(header_end + 4);
        std::cout << body << "\n";
    } else {
        std::cout << content << "\n";
    }
}

int main(int argc, char *argv[]) {
    const char *direccion = "127.0.0.1";
    const int puerto = 8080;
    
    std::string args;
    for (int i = 1; i < argc; i++) {
        args += std::string(argv[i]) + " ";
    }

    std::string command = "GET";
    std::string filename;
    std::string filecontent;

    if (args.find("--list") != std::string::npos) {
        command = "LIST";
    } 
    else if (args.find("--add=") != std::string::npos) {
        command = "ADD";
        size_t pos = args.find("--add=");
        size_t end = args.find(" ", pos);
        filename = args.substr(pos + 6, end - (pos + 6));

        std::string path = "Figuras/" + filename;
        std::cout << "Abriendo archivo en ruta: " << path << "\n";

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[ERROR] No se pudo abrir el archivo: " << filename << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        filecontent = buffer.str();
        file.close();
        
        std::cout << "[INFO] Contenido leído (" << filecontent.size() << " bytes)\n";
    }
    else if (args.find("--rm=") != std::string::npos) {
        // DELETE de adorno no termine
        std::cerr << "[ERROR] Comando DELETE no disponible\n";
        return 1;
    }
    else {
        size_t pos = args.find("--get=");
        if (pos != std::string::npos) {
            size_t end = args.find(" ", pos);
            filename = args.substr(pos + 6, end - (pos + 6));
        } else {
            filename = "killerWhale.txt";
        }
    }

    std::string request;
    if (command == "LIST") {
        request = "LIST /BEGIN//END/\n";
    } 
    else if (command == "GET") {
        request = "GET /BEGIN/" + filename + "/END/\n";
    } 
    else if (command == "ADD") {
        request = "ADD /BEGIN/" + filename + "/" + 
                 std::to_string(filecontent.size()) + "/END/\n" + filecontent;
    }

    const char *solicitud = request.c_str();
    std::cout << "\n================================\n";
    std::cout << "Conectando a: " << direccion << ":" << puerto << "\n";
    std::cout << "Comando: " << command << "\n";
    if (!filename.empty()) {
        std::cout << "Archivo: " << filename << "\n";
    }
    std::cout << "================================\n\n";

    VSocket *s = nullptr;
    char buffer[buffersize];
    memset(buffer, 0, buffersize);

    try {
        s = new Socket('s');
        s->MakeConnection(direccion, puerto);
        s->Write(solicitud, strlen(solicitud));

        std::cout << "[INFO] Solicitud enviada, esperando respuesta...\n";
        
        // Leer respuesta completa
        size_t total_bytes = 0;
        std::string full_response;
        
        while (true) {
            size_t bytes = s->Read(buffer, buffersize - 1);
            if (bytes == 0) break;
            
            buffer[bytes] = '\0';
            full_response.append(buffer, bytes);
            total_bytes += bytes;
            
            // Verificar si ya tenemos toda la respuesta
            if (full_response.find("HTTP/") != std::string::npos) {
                // Es HTTP, verificar si ya tenemos todo
                if (full_response.find("\r\n\r\n") != std::string::npos) {
                    // Verificar Content-Length
                    size_t cl_pos = full_response.find("Content-Length:");
                    if (cl_pos != std::string::npos) {
                        size_t cl_end = full_response.find("\r\n", cl_pos);
                        std::string cl_str = full_response.substr(cl_pos + 15, cl_end - (cl_pos + 15));
                        size_t content_length = std::stoul(cl_str);
                        
                        size_t header_end = full_response.find("\r\n\r\n");
                        size_t body_received = full_response.length() - header_end - 4;
                        
                        if (body_received >= content_length) {
                            break;
                        }
                    } else {
                        // No hay Content-Length
                        break;
                    }
                }
            } 
            else if (full_response.size() >= 6 && 
                    full_response.substr(full_response.size() - 6) == "/END/\n") {
                // Es protocolo y ya terminó
                break;
            }
        }
        
        std::cout << "[INFO] Respuesta recibida (" << total_bytes << " bytes)\n";
        
        // Determinar tipo de respuesta
        bool is_http = (full_response.find("HTTP/") != std::string::npos);
        
        if (is_http) {
            // Extraer código de estado HTTP
            size_t status_pos = full_response.find(" ");
            size_t status_end = full_response.find(" ", status_pos + 1);
            std::string status_code = full_response.substr(status_pos + 1, status_end - status_pos - 1);
            
            if (status_code == "200") {
                std::cout << "\n Operación exitosa (HTTP 200 OK)\n";
                
                if (command == "LIST") {
                    displayHTTPResponse(full_response);
                } else if (command == "GET") {
                    displayHTTPResponse(full_response);
                } else if (command == "ADD") {
                    std::cout << "\n Figura agregada exitosamente\n";
                }
            } else {
                std::cout << "\n Error HTTP " << status_code << "\n";
                displayHTTPResponse(full_response);
            }
        } else {
            // Protocolo
            if (full_response.find("/200/") != std::string::npos) {
                std::cout << "\n Operación exitosa\n";
                
                if (command == "LIST") {
                    displayProtocolFiguras(full_response);
                } else if (command == "GET") {
                    displayProtocolFiguraContent(full_response);
                } else if (command == "ADD") {
                    std::cout << "\n Figura agregada exitosamente\n";
                }
            } else if (full_response.find("/404/") != std::string::npos) {
                std::cout << "\n Error: Figura no encontrada\n";
            } else if (full_response.find("/400/") != std::string::npos) {
                std::cout << "\n Error: Solicitud inválida\n";
            } else if (full_response.find("/500/") != std::string::npos) {
                std::cout << "\n Error interno del servidor\n";
            } else {
                std::cout << "\n Respuesta desconocida:\n" << full_response << "\n";
            }
        }
        
    } catch (const std::runtime_error &e) {
        std::cerr << "\n Error de conexión: " << e.what() << "\n";
        std::cerr << "[INFO] Verifica que el intermediario esté ejecutándose\n";
    } catch (const std::exception &e) {
        std::cerr << "\n Error: " << e.what() << "\n";
    }

    if (s) {
        delete s;
    }
    
    return 0;
}