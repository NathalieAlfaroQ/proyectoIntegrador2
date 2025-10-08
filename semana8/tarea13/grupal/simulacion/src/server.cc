#include <cstring>
#include <iostream>
#include <unistd.h>
#include <regex>
#include <string>
#include <vector>

#include "FileSystem.hpp"

void servidor(int receive, int send) {
    std::vector<char> request_msg(4096);
    FileSystem fs;
    fs.buildFileSystem();
    std::regex requestRegex(R"((GET|ADD|DELETE|LIST) BEGIN/([\s\S]*?)/END\n([\s\S]*))");
    std::vector<char> response_msg(4096);

    while (true) {
        // Limpiar buffers
        std::fill(request_msg.begin(), request_msg.end(), 0);
        std::fill(response_msg.begin(), response_msg.end(), 0);
        
        // Leer mensaje - leer como buffer binario
        ssize_t bytes_read = read(receive, request_msg.data(), request_msg.size() - 1);
        if (bytes_read <= 0) {
            std::cout << "Server: Client disconnected" << std::endl;
            break;
        }
        
        // Convertir a string MANUALMENTE, preservando null bytes
        std::string request_str;
        request_str.assign(request_msg.data(), bytes_read);
        
        std::cout << "Server: received " << bytes_read << " bytes" << std::endl;

        // Verificar shutdown (solo verificar el inicio del string)
        if (request_str.find("BEGIN/OFF/CLIENTE") == 0) {
            printf("Server: received shutdown message\n");
            break;
        }

        std::string response;
        std::smatch match;
        
        // Procesar el mensaje con el regex
        if (std::regex_match(request_str, match, requestRegex)) {
            std::string comando = match[1];
            std::string metadata = match[2];
            std::string contenido = match[3];  // Este contiene el arte ASCII con null bytes
            
            std::cout << "Server: Command: " << comando << ", Metadata: " << metadata 
                      << ", Content length: " << contenido.size() << std::endl;

            std::ostringstream oss;
            if (comando == "ADD") {
                // Extraer nombre del archivo y tamaño
                std::regex fileNameRegex(R"((.+?)\.txt/contentLength(\d+))");
                std::smatch nameMatch;

                if (std::regex_match(metadata, nameMatch, fileNameRegex)) {
                    std::string fileName = nameMatch[1];
                    std::string contentLengthStr = nameMatch[2];
                    
                    std::cout << "Server: Adding file: " << fileName 
                              << ", size: " << contentLengthStr << std::endl;
                    
                    // Crear un stringstream desde el contenido binario
                    std::istringstream cuerpo(contenido);
                    
                    if (fs.addFile(fileName + ".txt", cuerpo) == 0) {
                        oss << "BEGIN/200 OK/added:" << fileName << "/END\n";
                    } else {
                        oss << "BEGIN/500 Unable to add file/END\n";
                    }
                } else {
                    oss << "BEGIN/400 Invalid ADD format/END\n";
                }
                
            } else if (comando == "LIST") {
                std::string dir = fs.getDirectory();
                oss << "BEGIN/200 OK/contentLength" << dir.size() << "/END\n" << dir;

            } else if (comando == "DELETE") {
                if (fs.deleteFile(metadata + ".txt") == 0) {
                    oss << "BEGIN/200 OK/Figure deleted successfully/END\n";
                } else {
                    oss << "BEGIN/500 Error at deleting file/END\n";
                }
                
            } else if (comando == "GET") {
                std::string file = fs.getFile(metadata + ".txt");
                if (file.empty()) {
                    oss << "BEGIN/404 File Not Found/END\n";
                } else {
                    oss << "BEGIN/200 OK/contentLength:" << file.size() << "/END\n" << file;
                }

            } else {
                response = "BEGIN/400 Bad Request/END\n";
            }
            response = oss.str();

        } else {
            std::cout << "Server: Request no coincide con el regex" << std::endl;
            response = "BEGIN/400 Bad Request/END\n";
        }

        // Enviar respuesta
        std::cout << "Server: Sending response: " << response.substr(0, 50) << std::endl;
        strncpy(response_msg.data(), response.c_str(), response_msg.size() - 1);
        response_msg[response_msg.size() - 1] = '\0';
        write(send, response.c_str(), response.size());
    }
}