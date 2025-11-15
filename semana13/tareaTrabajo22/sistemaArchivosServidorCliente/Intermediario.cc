#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include "Socket.h"

using namespace std;

int main() {
    Socket intermediario('s');
    if (intermediario.Bind(9090) < 0) {
        cerr << "[INTERMEDIARIO] Error al bindear puerto 9090" << endl;
        return 1;
    }
    intermediario.MarkPassive(10);

    cout << "[INTERMEDIARIO] Intermediario escuchando en puerto 9090..." << endl;
    cout << "[INTERMEDIARIO] Redirigiendo a Servidor HTTP en puerto 8080" << endl;
    cout << "[INTERMEDIARIO] URL para navegador: http://127.0.0.1:9090/listado" << endl;

    while (true) {
        VSocket *cliente = intermediario.AcceptConnection();
        cout << "\n[INTERMEDIARIO] Cliente conectado" << endl;

        char buffer[8192];  // Buffer más grande para solicitudes HTTP completas
        int n = cliente->Read(buffer, sizeof(buffer) - 1);

        if (n <= 0) {
            cerr << "[INTERMEDIARIO] Error leyendo solicitud" << endl;
            delete cliente;
            continue;
        }

        buffer[n] = '\0';
        string solicitud_cliente(buffer, n);
        
        cout << "[INTERMEDIARIO] Solicitud HTTP recibida (" << n << " bytes)" << endl;
        
        // Mostrar solo las primeras líneas para debugging
        istringstream iss(solicitud_cliente);
        string linea;
        int line_count = 0;
        while (getline(iss, linea) && line_count < 5) {
            cout << "[INTERMEDIARIO] " << linea << endl;
            line_count++;
        }

        // Reenviar la solicitud al Servidor HTTP
        Socket servidor_http('s');
        if (servidor_http.MakeConnection("127.0.0.1", 8080) < 0) {
            cerr << "[INTERMEDIARIO] Error conectando al Servidor HTTP" << endl;
            string error_response = "HTTP/1.1 500 Internal Server Error\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Connection: close\r\n"
                                  "\r\n"
                                  "Error: No se puede conectar al servidor backend";
            cliente->Write(error_response.c_str());
            delete cliente;
            continue;
        }

        cout << "[INTERMEDIARIO] Conectado al Servidor HTTP, reenviando solicitud..." << endl;

        // Enviar solicitud al Servidor HTTP
        servidor_http.Write(solicitud_cliente.c_str());

        // Recibir respuesta del Servidor HTTP y enviarla al cliente
        char buffer_respuesta[8192];
        string respuesta_completa;
        int total_bytes = 0;
        
        try {
            while (true) {
                memset(buffer_respuesta, 0, sizeof(buffer_respuesta));
                int bytes_leidos = servidor_http.Read(buffer_respuesta, sizeof(buffer_respuesta) - 1);
                
                if (bytes_leidos <= 0) {
                    break;
                }
                
                // Enviar al cliente inmediatamente
                cliente->Write(buffer_respuesta, bytes_leidos);
                total_bytes += bytes_leidos;
                
                // Si es el inicio de la respuesta, mostrarlo para debugging
                if (respuesta_completa.empty() && bytes_leidos > 0) {
                    respuesta_completa = string(buffer_respuesta, min(bytes_leidos, 200));
                    cout << "[INTERMEDIARIO] Inicio de respuesta: " 
                         << respuesta_completa.substr(0, 100) << "..." << endl;
                }
            }
            
            cout << "[INTERMEDIARIO] Respuesta enviada al cliente (" << total_bytes << " bytes)" << endl;
            
        } catch (std::exception& ex) {
            cout << "[INTERMEDIARIO] Error durante transferencia: " << ex.what() << endl;
        }

        // Cerrar conexiones
        servidor_http.Close();
        delete cliente;
        
        cout << "[INTERMEDIARIO] Conexión cerrada" << endl;
    }
    
    return 0;
}