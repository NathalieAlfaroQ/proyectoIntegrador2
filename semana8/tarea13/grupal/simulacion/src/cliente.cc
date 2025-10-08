#include <cstring>
#include <iostream>
#include <unistd.h> // pipes
#include <iostream>
#include <string>
#include <regex>
#include <sstream>
#include <fstream>
    #include <limits>

void cliente(int receive, int send) {
    std::vector<char> response_msg(4096);

    while(true) {
        std::fill(response_msg.begin(), response_msg.end(), 0);

        std::cout << "Ingrese la operación por realizar:\nGET\nADD\nLIST\nEXIT\nDELETE\n>> ";
        std::string instr;
        std::cin >> instr;
        std::regex instruccionRegex(R"((GET|ADD|LIST|DELETE|EXIT))");
        std::smatch match;

        if (std::regex_search(instr, match, instruccionRegex)) {
            std::string instruccion = match[0];
            std::string metadata = "";
            std::string content = "";

            if (instruccion == "GET") {
                std::cout << "Ingrese el nombre de la figura sin la extensión\n>> ";
                std::cin >> metadata;

            } else if (instruccion == "LIST") {
                metadata = "listFigures";

            } else if (instruccion == "ADD") {
                std::cout << "Ingrese el nombre de la figura sin la extensión\n>> ";
                std::string nombreFigura;
                std::cin >> nombreFigura;
                std::ifstream inputFile("files/" + nombreFigura + ".txt", std::ios::binary);

                if (!inputFile.is_open()) {
                    std::cout << "Archivo no encontrado. Cambiando a LIST.\n";
                    instruccion = "LIST";
                    metadata = "listFigures";
                } else {
                    std::stringstream fileBuffer;
                    fileBuffer << inputFile.rdbuf();
                    content = fileBuffer.str();
                    metadata = nombreFigura + ".txt/contentLength" + std::to_string(content.size());
                }

            } else if (instruccion == "DELETE") {
                std::cout << "Ingrese el nombre de la figura sin la extensión\n>> ";
                std::cin >> metadata;

            } else if (instruccion == "EXIT") {
                std::string shutDownStr = "BEGIN/OFF/CLIENTE/IP/port/END";
                write(send, shutDownStr.c_str(), shutDownStr.size());
                break;
            }

            std::ostringstream request;
            request << instruccion << " BEGIN/" << metadata << "/END\n" << content;
            std::string requestMsg = request.str();
            
            printf("* Client sending: %s...\n", requestMsg.c_str());
            
            // Enviar TODOS los bytes, no solo hasta el primer null
            write(send, requestMsg.c_str(), requestMsg.size());

            // Leer respuesta
            ssize_t bytes_read = read(receive, response_msg.data(), response_msg.size() - 1);
            if (bytes_read > 0) {
                response_msg[bytes_read] = '\0';
                printf("* Client received: \n%s\n", response_msg.data());
            } else {
                printf("* Client received empty response\n");
            }

        } else {
            std::cout << "Instrucción no válida" << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}