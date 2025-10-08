#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

void tenedor(int receiveClient, int sendServer, int receiveServer, int sendClient) {
    std::vector<char> request_msg(4096);
    std::vector<char> response_msg(4096);
    
    while (true) {
        // Limpiar buffers completamente
        std::fill(request_msg.begin(), request_msg.end(), 0);
        std::fill(response_msg.begin(), response_msg.end(), 0);
        
        // Leer del cliente - usar read() que maneja null bytes
        ssize_t request_bytes = read(receiveClient, request_msg.data(), request_msg.size() - 1);
        if (request_bytes <= 0) {
            std::cout << "* Fork: Client disconnected" << std::endl;
            break;
        }
        
        printf("* Fork received (client request): %s\n", request_msg.data()); // Solo imprimir primeros 100 chars

        // Enviar al servidor - usar el número exacto de bytes leídos
        write(sendServer, request_msg.data(), request_bytes);
        
        // Leer respuesta del servidor
        ssize_t response_bytes = read(receiveServer, response_msg.data(), response_msg.size() - 1);
        if (response_bytes <= 0) {
            std::cout << "* Fork: Server disconnected" << std::endl;
            break;
        }
        
        printf("* Fork received (server response): \n%.200s\n", response_msg.data()); // Solo primeros 200 chars

        // Enviar respuesta al cliente - usar el número exacto de bytes
        write(sendClient, response_msg.data(), response_bytes);
    }
}