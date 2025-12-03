#include <thread>
#include <cstring> // Define memset

#include "ForkRouter.h"
#include "ProtocoloGrupal.h"
#include "Socket.h"
#include "Utils.h"


/**
 * @brief Para un hilo. Se encarga de recibir y guardar conexiones de otros
 * tenedores.
 * @param routerMem La memoria compartida del router, Thread-safe.
 */
void handleServerConnect(ForkRouter* routerMem);
/**
 * @brief Acepta conexiones de forks indefinidamente. Responde con un connect
 * cuando recibe una conexión nueva.
 * @param routerMem La memoria compartida del router, Thread-safe.
 */
void handleForkConnect(ForkRouter* routerMem);
/**
 * @brief Acepta conexiones del server. Hace handshake con el server y guarda
 * su IP. Duerme hasta que el server sea eliminado.
 * @param routerMem La memoria compartida del router, Thread-safe.
 */
void sendInitialUpdate(ForkRouter* routerMem, std::string forkIP);
void serveRequest(ForkRouter* routerMem);
/**
 * @brief Rutea los mensaje HTTP del mensaje.
 * @param routerMem La memoria compartida del router, Thread-safe.
 * @param parsedRequest La request parseada del mensaje
 */
std::string routeMessage(ForkRouter* routerMem, std::string request);
std::string askServer(ForkRouter* routerMem, const std::string& message);
std::string askFork(ForkRouter* routerMem, const std::string& message, const std::string& ip);

int main(int argc, char const *argv[]) {
    // Inicializar variables comunes
    std::string localForkIP = getLocalIP("enp0s20f0u1");
    ForkRouter routerMem(localForkIP);

    // Crear hilos de conexión
    std::thread *serverWorker = new std::thread(handleServerConnect, &routerMem);	// service connection
    std::thread *forkWorker = new std::thread(handleForkConnect, &routerMem);	// service connection

    // Crear socket de atención
    int portToListen = 8080;
    Socket other('s');
    other.Bind(portToListen);
    other.MarkPassive(5);

    // Atender solicitudes con el hilo principal
    while (true) {
        Socket *client = (Socket *) other.AcceptConnection();
        routerMem.currentConnection = client;
        printf("Connection request received!\n\n");
        std::thread *worker = new std::thread(serveRequest, &routerMem);	// service connection
    }

    /* code */
    return 0;
}

void handleServerConnect(ForkRouter* routerMem) {
    int toServerPort = 1234;

    while (true) {
        std::string serverIP;
        bool ok = handshake(serverIP, routerMem->myHostname, toServerPort, "enp0s20f0u1");
        if (!ok) {
            std::cerr << "Error: no logró hacer un handshake exitoso." << std::endl;
        } else {
            // Esta línea duerme al hilo hasta que el servidor se elimine
            std::cout << "[TCP] Pidiendo al server sus figuras." << std::endl;

            std::string request = "LIST /BEGIN//END/";

            std::string response = askServer(routerMem, request);

            std::cout << "[TCP] Respuesta del tenedor por update: " << response << std::endl;

            struct PG_Message parsedResponse = parseAndVaildatePG_Request(response);

            routerMem->addServer(routerMem->myHostname, serverIP, split(parsedResponse.cuerpo, "\n"));
        }
    }
}

void handleForkConnect(ForkRouter* routerMem) {
    int toForkPort = 4321;
    const std::vector<std::string> broadcastAddresses = {
        "172.16.123.15"
        , "172.16.123.31"
        , "172.16.123.47"
        , "172.16.123.63"
        , "172.16.123.79"
        , "172.16.123.95"
        , "172.16.123.111"
    };

    std::string headers = routerMem->myIP + "/" + routerMem->myHostname;
    std::string connectMsg = buildPG_Request("CONNECT", headers, "");

    // Se realiza broadcast con la dirección al encender
    int status = sendAnyway(connectMsg, broadcastAddresses, toForkPort, false);

    while (true) {
        Socket server('d');
        server.Bind(toForkPort);

        // Paquete del mensaje
        char buf[65536];
        struct sockaddr_storage from;

        // Leer respuesta
        memset(&from, 0, sizeof(from));
        ssize_t n = server.recvFrom(buf, sizeof(buf), (void*)&from);
        if (n <= 0) {
            printf("recvFrom returned %zd. No message received.\n", n);
            throw std::runtime_error("No response received\n");
        }

        // Validar respuesta
        std::string response = std::string(buf, n);
        std::cout << "[UDP] Mensaje recibido de un fork: " << response << std::endl;
        struct PG_Message parsedResponse = parseAndVaildatePG_Request(response);

        if (parsedResponse.method == "CONNECT") {
            if (routerMem->addFork(parsedResponse.hostname, parsedResponse.ip, split(parsedResponse.cuerpo, "\n"))) {
                // Se agregó con éxito
                int status = sendAnyway(connectMsg, broadcastAddresses, toForkPort, false);
                sendInitialUpdate(routerMem, parsedResponse.ip);
            } else {
                // Ya existía el servidor
                // No hace nada
            }
        }
    }
}

// Una vez conectado, debe enviar un UPDATE con las figuras del server, si hay
void sendInitialUpdate(ForkRouter* routerMem, std::string forkIP) {
    int port = 8080;
    std::string serverList = routerMem->getServerList();
    std::string header = "C/" + routerMem->myHostname + "/" + std::to_string(serverList.size());
    std::string request = buildPG_Request("UPDATE", header, serverList);
    Socket s('s');
    s.MakeConnection(forkIP.c_str(), port);
    s.Write(request.c_str(), request.size());

    std::string response;
    readVerify(s, response);
    std::cout << "Respuesta del tenedor por update: " << response << std::endl;
}

void serveRequest(ForkRouter* routerMem) {
        // Recibir mensaje
        std::string request;
        std::string response;

        // Valida el mensaje y crea la respuesta
        bool isHTTP = readVerify(*(routerMem->currentConnection), request);

        std::cout << "[TCP] Mensaje recibido: " << request << std::endl;

        if (routerMem->isConnected()) {
            std::cout << "[TCP] Hay conexión a un componente." << std::endl;
            if (isHTTP) {
                std::cout << "[TCP] Es del navegador." << std::endl;
                bool isList = request.find("LIST") != std::string::npos;
                response = rRequestHTTP(request, isList);
            } else {
                std::cout << "[TCP] NO es del navegador." << std::endl;
                response = routeMessage(routerMem, request);
            }
        } else {
            // Construir mensaje de error - No hay servers disponibles
            std::string html =
                "<html>\n<body>\n"
                "<h1>Internal Error 500</h1>"
                "<p>El tenedor no tiene conexiones abiertas.</p>"
                "</html>\n</body>\n";
            if (isHTTP) {
                response =
                    "HTTP/1.1 500 Internal Error\r\n"
                    "Content-Type: text/html; charset=UTF-8\r\n"
                    "Content-Length: " + std::to_string(html.size()) + "\r\n"
                    "\r\n" +
                    html;
            } else {
                response = buildPG_Response(500, html);
            }
        }

        // Enviar respuesta
        std::cout << "[TCP] Respuesta enviada: " << response << std::endl;

        routerMem->currentConnection->Write(response.c_str());
}

std::string routeMessage(ForkRouter* routerMem, std::string request) {
    struct PG_Message parsedRequest = parseAndVaildatePG_Request(request);
    std::string response;

    // Manejar los casos ADD, DELETE, GET, LIST, QUIT, UPDATE
    if (parsedRequest.method == "ADD") {
        response = askServer(routerMem, request);
    } else if (parsedRequest.method == "DELETE") {
        response = askServer(routerMem, request);
    } else if (parsedRequest.method == "GET") {
        std::string ip = routerMem->findFigure(parsedRequest.filename);
        if (ip == "") {
            // Preguntarle al server
            response = askServer(routerMem, request);
        } else {
            // Preguntarle al fork que tiene la figura
            response = askFork(routerMem, request, ip);
        }
    } else if (parsedRequest.method == "LIST") {
        std::string list = routerMem->getEveryoneList();
        response = buildPG_Response(200, list);
    } else if (parsedRequest.method == "QUIT") {
        routerMem->removeFork(parsedRequest.ip);
        response = buildPG_Response(200, "");
    } else if (parsedRequest.method == "UPDATE") {
        if (parsedRequest.operation = 'C') {
            for (std::string fileName : split(parsedRequest.cuerpo, "\n")) {
                routerMem->addFigure(parsedRequest.hostname, fileName);
            }
        } else if (parsedRequest.operation = 'Q') {
            std::string forkIP = routerMem->findHost(parsedRequest.hostname);
            routerMem->removeFork(forkIP);
        } else if (parsedRequest.operation = 'A') {
            routerMem->addFigure(parsedRequest.hostname, parsedRequest.cuerpo);
        } if (parsedRequest.operation = 'D') {
            routerMem->deleteFigure(parsedRequest.hostname, parsedRequest.cuerpo);
        } else {
            std::cerr << "[TCP] Error del Update, no se reconoce la operación: "
            << parsedRequest.operation << ".\n";
        }
        response = buildPG_Response(200, "");
    }

    return response;
}

std::string askServer(ForkRouter* routerMem, const std::string& message) {
    int port = 8081;
    std::string response;

    std::string serverIP = routerMem->getServerIP();
    try {
        Socket s('s');
        s.MakeConnection(serverIP.c_str(), port);
        s.Write(message.c_str(), message.size());

        readVerify(s, response);
    } catch (const std::exception& e) {
        std::cerr << "[TCP] Error enviando a " << serverIP.c_str() << ": " << e.what() << "\n";
    }

    return response;
}

std::string askFork(ForkRouter* routerMem, const std::string& message, const std::string& ip) {
    int port = 8080;
    std::string response;

    try {
        Socket s('s');
        s.MakeConnection(ip.c_str(), port);
        s.Write(message.c_str(), message.size());

        readVerify(s, response);
    } catch (const std::exception& e) {
        std::cerr << "[TCP] Error enviando a " << ip.c_str() << ": " << e.what() << "\n";
    }

    return response;
}