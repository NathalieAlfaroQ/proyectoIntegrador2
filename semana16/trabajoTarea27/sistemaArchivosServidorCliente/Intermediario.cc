#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <csignal>
#include <atomic>
#include <regex>
#include <unordered_map>
#include <vector>
#include <set>
#include "VSocket.h"
#include "Socket.h"
#include "Utils.h"

#define BUFFER_SIZE 4096

// Estructura para información de servidores conectados
struct ServerNode {
    std::string hostname;
    std::string ip;
    int tcp_port;
    std::set<std::string> figuras;
    bool online;
    time_t last_seen;
};

class NetworkRouter {
private:
    std::mutex router_mutex;
    std::unordered_map<std::string, ServerNode> servers;
    std::unordered_map<std::string, std::string> figura_location;
    std::string my_hostname;
    std::string my_ip;
    std::string main_server_ip;
    int main_server_port;
    
public:
    NetworkRouter(const std::string& hostname) 
        : my_hostname(hostname), main_server_port(8081) {
        my_ip = getLocalIP();
    }
    
    void setMainServer(const std::string& ip) {
        std::lock_guard<std::mutex> lock(router_mutex);
        main_server_ip = ip;
        
        ServerNode main_node;
        main_node.hostname = "main_server";
        main_node.ip = ip;
        main_node.tcp_port = main_server_port;
        main_node.online = true;
        main_node.last_seen = time(nullptr);
        servers["main_server"] = main_node;
        
        std::cout << "[ROUTER] Servidor principal: " << ip << "\n";
    }
    
    void addNode(const std::string& hostname, const std::string& ip, int port = 8080) {
        std::lock_guard<std::mutex> lock(router_mutex);
        
        if (servers.find(hostname) == servers.end()) {
            ServerNode node;
            node.hostname = hostname;
            node.ip = ip;
            node.tcp_port = port;
            node.online = true;
            node.last_seen = time(nullptr);
            servers[hostname] = node;
            
            std::cout << "[ROUTER] Nodo agregado: " << hostname 
                     << " (" << ip << ":" << port << ")\n";
        } else {
            servers[hostname].ip = ip;
            servers[hostname].tcp_port = port;
            servers[hostname].online = true;
            servers[hostname].last_seen = time(nullptr);
        }
    }
    
    void removeNode(const std::string& hostname) {
        std::lock_guard<std::mutex> lock(router_mutex);
        
        if (servers.find(hostname) != servers.end()) {
            auto& figuras = servers[hostname].figuras;
            for (const auto& figura : figuras) {
                figura_location.erase(figura);
            }
            servers.erase(hostname);
            std::cout << "[ROUTER] Nodo removido: " << hostname << "\n";
        }
    }
    
    void addFiguraToNode(const std::string& hostname, const std::string& figura) {
        std::lock_guard<std::mutex> lock(router_mutex);
        
        if (servers.find(hostname) != servers.end()) {
            servers[hostname].figuras.insert(figura);
            figura_location[figura] = hostname;
            std::cout << "[ROUTER] Figura asignada: " << figura << " a " << hostname << "\n";
        }
    }
    
    std::pair<std::string, int> findFiguraLocation(const std::string& figura) {
        std::lock_guard<std::mutex> lock(router_mutex);
        
        auto it = figura_location.find(figura);
        if (it != figura_location.end()) {
            const ServerNode& node = servers[it->second];
            return {node.ip, node.tcp_port};
        }
        return {main_server_ip, main_server_port};
    }
    
    std::pair<std::string, int> findLeastLoadedNode() {
        std::lock_guard<std::mutex> lock(router_mutex);
        
        std::string best_hostname = "main_server";
        size_t min_figuras = servers["main_server"].figuras.size();
        
        for (const auto& pair : servers) {
            if (pair.second.online && pair.first != "main_server") {
                if (pair.second.figuras.size() < min_figuras) {
                    min_figuras = pair.second.figuras.size();
                    best_hostname = pair.first;
                }
            }
        }
        
        const ServerNode& best = servers[best_hostname];
        return {best.ip, best.tcp_port};
    }
    
    void processFiguraList(const std::string& hostname, const std::string& list_str) {
        std::lock_guard<std::mutex> lock(router_mutex);
        
        if (servers.find(hostname) == servers.end()) return;
        
        auto& node = servers[hostname];
        node.figuras.clear();
        
        std::vector<std::string> figuras = split(list_str, "\n");
        for (const auto& figura : figuras) {
            if (!figura.empty() && figura != "\n" && figura != "\r") {
                std::string clean_figura = figura;
                if (!clean_figura.empty() && clean_figura.back() == '\r') {
                    clean_figura.pop_back();
                }
                if (!clean_figura.empty()) {
                    node.figuras.insert(clean_figura);
                    figura_location[clean_figura] = hostname;
                }
            }
        }
        
        std::cout << "[ROUTER] " << node.figuras.size() << " figuras de " << hostname << "\n";
    }
    
    std::string getAllFiguras() {
        std::lock_guard<std::mutex> lock(router_mutex);
        std::string result;
        for (const auto& pair : figura_location) {
            result += pair.first + "\n";
        }
        return result;
    }
    
    std::vector<std::string> getAllActiveIPs() {
        std::lock_guard<std::mutex> lock(router_mutex);
        std::vector<std::string> ips;
        for (const auto& pair : servers) {
            if (pair.second.online && pair.first != "main_server") {
                ips.push_back(pair.second.ip);
            }
        }
        return ips;
    }
    
    std::string getMyHostname() const { return my_hostname; }
    std::string getMyIP() const { return my_ip; }
    std::string getMainServerIP() const { return main_server_ip; }
};

NetworkRouter* network_router = nullptr;
std::atomic<bool> is_running(true);
std::mutex recovery_mutex;

void signal_handler(int) {
    is_running.store(false);
    std::cout << "\n[INFO] Recibida señal de terminación\n";
}

void udpBroadcastListener() {
    try {
        Socket udp_socket('d');
        udp_socket.Bind(4321);
        
        std::cout << "[UDP-BROADCAST] Escuchando en puerto 4321\n";
        
        while (is_running.load()) {
            std::string message;
            sockaddr_in sender{};
            
            if (waitMessage(udp_socket, 1, message, sender)) {
                char sender_ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &sender.sin_addr, sender_ip_str, sizeof(sender_ip_str));
                std::string sender_ip = sender_ip_str;
                
                std::regex connect_pattern("CONNECT\\s/BEGIN/([\\d\\.]+)/([\\w\\.-]+)/END/");
                std::smatch match;
                
                if (std::regex_search(message, match, connect_pattern)) {
                    std::string remote_ip = match[1];
                    std::string remote_hostname = match[2];
                    
                    if (remote_ip == network_router->getMyIP() || 
                        remote_hostname == network_router->getMyHostname()) {
                        continue;
                    }
                    
                    network_router->addNode(remote_hostname, remote_ip, 8080);
                    
                    std::string response = "CONNECT /BEGIN/" + 
                                          network_router->getMyIP() + "/" + 
                                          network_router->getMyHostname() + "/END/\n";
                    
                    sockaddr_in response_addr{};
                    response_addr.sin_family = AF_INET;
                    response_addr.sin_port = htons(4321);
                    inet_aton(remote_ip.c_str(), &response_addr.sin_addr);
                    
                    udp_socket.sendTo(response.c_str(), response.size(), &response_addr);
                    
                    std::cout << "[UDP-BROADCAST] Respuesta enviada a " << remote_ip << "\n";
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[UDP-BROADCAST ERROR] " << e.what() << "\n";
    }
}

void udpServerCommunicator() {
    try {
        Socket udp_socket('d');
        udp_socket.Bind(1234);
        
        std::cout << "[UDP-SERVER] Listo en puerto 1234\n";
        
        while (is_running.load()) {
            std::string message;
            sockaddr_in sender{};
            
            if (waitMessage(udp_socket, 2, message, sender)) {
                char sender_ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &sender.sin_addr, sender_ip_str, sizeof(sender_ip_str));
                std::string sender_ip = sender_ip_str;
                
                if (message.find("/BEGIN/200/END/") != std::string::npos) {
                    std::cout << "[UDP-SERVER] Handshake confirmado con servidor\n";
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[UDP-SERVER ERROR] " << e.what() << "\n";
    }
}

void discoverNetwork() {
    try {
        std::string my_ip = network_router->getMyIP();
        std::string my_hostname = network_router->getMyHostname();
        std::string interface_name = getInterfaceName();
        std::string broadcast_addr = getBroadcastIP(interface_name);
        
        Socket broadcast_socket('d');
        int broadcast_opt = 1;
        setsockopt(broadcast_socket.returnFd(), SOL_SOCKET, SO_BROADCAST, &broadcast_opt, sizeof(broadcast_opt));
        
        sockaddr_in broadcast_dest{};
        broadcast_dest.sin_family = AF_INET;
        broadcast_dest.sin_port = htons(4321);
        broadcast_dest.sin_addr.s_addr = inet_addr(broadcast_addr.c_str());
        
        std::string connect_message = "CONNECT /BEGIN/" + my_ip + "/" + my_hostname + "/END/\n";
        
        for (int attempt = 0; attempt < 3 && is_running.load(); attempt++) {
            broadcast_socket.sendTo(connect_message.c_str(), connect_message.size(), &broadcast_dest);
            std::cout << "[DISCOVERY] Broadcast enviado a " << broadcast_addr << "\n";
            for (int i = 0; i < 10 && is_running.load(); i++) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[DISCOVERY ERROR] " << e.what() << "\n";
    }
}

void handleClientRequest(VSocket* client_socket) {
    std::string client_request;
    
    try {
        bool is_protocol = readVerify(*client_socket, client_request);
        
        std::cout << "[CLIENT] Solicitud recibida\n";
        
        std::string response;
        
        bool is_LIST = client_request.find("LIST") != std::string::npos;
        bool is_GET = client_request.find("GET") != std::string::npos;
        bool is_ADD = client_request.find("ADD") != std::string::npos;
        bool is_UPDATE = client_request.find("UPDATE") != std::string::npos;
        
        if (is_LIST) {
            try {
                Socket main_server('s');
                main_server.MakeConnection(network_router->getMainServerIP().c_str(), 8081);
                
                std::string list_cmd = "LIST /BEGIN//END/\n";
                main_server.Write(list_cmd.c_str(), list_cmd.size());
                
                std::string main_list_response;
                readVerify(main_server, main_list_response);
                
                std::regex list_re("/BEGIN/200/(\\d+)/END/\n([\\s\\S]*)");
                std::smatch list_match;
                
                if (std::regex_search(main_list_response, list_match, list_re)) {
                    std::string main_list = list_match[2];
                    std::string all_figuras = main_list + network_router->getAllFiguras();
                    
                    if (is_protocol) {
                        response = "/BEGIN/200/" + std::to_string(all_figuras.size()) + 
                                  "/END/\n" + all_figuras;
                    } else {
                    
                        response = "HTTP/1.1 200 OK\r\n";
                        response += "Content-Type: text/plain\r\n";
                        response += "Content-Length: " + std::to_string(all_figuras.size()) + "\r\n";
                        response += "\r\n" + all_figuras;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "[LIST ERROR] " << e.what() << "\n";
                response = is_protocol ? "/BEGIN/500/END/\n" : 
                    "HTTP/1.1 500 Internal Server Error\r\n\r\n";
            }
        }
        else if (is_GET) {
            std::regex get_re("GET\\s/BEGIN/([\\w\\.]+)/");
            std::smatch get_match;
            
            if (std::regex_search(client_request, get_match, get_re)) {
                std::string figura_name = get_match[1];
                
                auto [target_ip, target_port] = network_router->findFiguraLocation(figura_name);
                
                try {
                    Socket target_server('s');
                    target_server.MakeConnection(target_ip.c_str(), target_port);
                    target_server.Write(client_request.c_str(), client_request.size());
                    
                    std::string target_response;
                    readVerify(target_server, target_response);
                    
                    if (is_protocol) {
                        response = target_response;
                    } else {
                        // Convertir respuesta del protocolo a HTTP 
                        std::regex proto_re("/BEGIN/200/(\\d+)/END/\n([\\s\\S]*)");
                        std::smatch proto_match;
                        
                        if (std::regex_search(target_response, proto_match, proto_re)) {
                            std::string content = proto_match[2];
                            response = "HTTP/1.1 200 OK\r\n";
                            response += "Content-Type: text/plain\r\n";
                            response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
                            response += "\r\n" + content;
                        } else {
                            response = "HTTP/1.1 404 Not Found\r\n\r\n";
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[GET ERROR] " << e.what() << "\n";
                    response = is_protocol ? "/BEGIN/404/END/\n" : 
                        "HTTP/1.1 404 Not Found\r\n\r\n";
                }
            }
        }
        else if (is_ADD) {
            std::regex add_re("ADD\\s/BEGIN/([\\w\\.]+)/(\\d+)/END/\n([\\s\\S]*)");
            std::smatch add_match;
            
            if (std::regex_search(client_request, add_match, add_re)) {
                std::string figura_name = add_match[1];
                std::string figura_content = add_match[3];
                
                auto [target_ip, target_port] = network_router->findLeastLoadedNode();
                
                try {
                    Socket target_server('s');
                    target_server.MakeConnection(target_ip.c_str(), target_port);
                    target_server.Write(client_request.c_str(), client_request.size());
                    
                    std::string target_response;
                    readVerify(target_server, target_response);
                    
                    if (target_response.find("/200/") != std::string::npos) {
                        network_router->addFiguraToNode("main_server", figura_name);
                        
                        std::string update_msg = "UPDATE /BEGIN/A/Ballenitas/" +
                                                std::to_string(figura_name.size()) + 
                                                "/END/\n" + figura_name;
                        
                        std::vector<std::string> all_nodes = network_router->getAllActiveIPs();
                        sendAnyway(update_msg, all_nodes, 8080, true);
                    }
                    
                    response = is_protocol ? target_response : 
                              "HTTP/1.1 200 OK\r\n\r\nFigura agregada exitosamente";
                    
                } catch (const std::exception& e) {
                    std::cerr << "[ADD ERROR] " << e.what() << "\n";
                    response = is_protocol ? "/BEGIN/500/END/\n" : 
                        "HTTP/1.1 500 Internal Server Error\r\n\r\n";
                }
            }
        }
        else if (is_UPDATE) {
            std::regex update_re(R"(UPDATE\s/BEGIN/(.)/([\w\.-]+)/(\d+)/END/?([\s\\S]*))");
            std::smatch update_match;
            
            if (std::regex_search(client_request, update_match, update_re)) {
                char operation = update_match[1].str()[0];
                std::string source_host = update_match[2];
                std::string data = update_match[4];
                
                switch (operation) {
                    case 'A':
                        network_router->addFiguraToNode(source_host, data);
                        std::cout << "[UPDATE] Figura agregada: " << data << "\n";
                        break;
                    case 'C':
                        network_router->processFiguraList(source_host, data);
                        std::cout << "[UPDATE] Nodo conectado: " << source_host << "\n";
                        break;
                    case 'Q':
                        network_router->removeNode(source_host);
                        std::cout << "[UPDATE] Nodo desconectado: " << source_host << "\n";
                        break;
                }
                
                response = "/BEGIN/200/END/\n";
            } else {
                response = "/BEGIN/400/END/\n";
            }
        }
        else {
            response = is_protocol ? "/BEGIN/400/END/\n" : 
                "HTTP/1.1 400 Bad Request\r\n\r\n";
        }
        
        client_socket->Write(response.c_str(), response.size());
        
    } catch (const std::exception& e) {
        std::cerr << "[CLIENT HANDLER ERROR] " << e.what() << "\n";
    }
    
    delete client_socket;
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGPIPE, SIG_IGN);
    
    std::string my_hostname = "Ballenitas";
    
    std::cout << "========================================\n";
    std::cout << "INTERMEDIARIO: " << my_hostname << "\n";
    std::cout << "IP Local: " << getLocalIP() << "\n";
    std::cout << "========================================\n\n";
    
    network_router = new NetworkRouter(my_hostname);
    
    std::cout << "[MAIN] Buscando servidor principal...\n";
    std::string main_server_ip;
    if (handshake(main_server_ip, my_hostname, 1234)) {
        network_router->setMainServer(main_server_ip);
        std::cout << "[MAIN] Servidor principal: " << main_server_ip << "\n";
    } else {
        std::cout << "[MAIN] Usando localhost como fallback\n";
        network_router->setMainServer("127.0.0.1");
    }
    
    std::thread udp_broadcast_thread(udpBroadcastListener);
    std::thread udp_server_thread(udpServerCommunicator);
    std::thread discovery_thread(discoverNetwork);
    
    Socket tcp_server('s');
    tcp_server.Bind(8080);
    tcp_server.MarkPassive(10);
    
    std::cout << "\n[MAIN] Escuchando en puerto 8080\n";
    
    int server_fd = tcp_server.returnFd();
    
    while (is_running.load()) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(server_fd, &read_set);
        
        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int ready = select(server_fd + 1, &read_set, nullptr, nullptr, &timeout);
        
        if (ready > 0 && FD_ISSET(server_fd, &read_set)) {
            VSocket* client = tcp_server.AcceptConnection();
            if (client) {
                std::thread client_thread(handleClientRequest, client);
                client_thread.detach();
            }
        }
    }
    
    std::cout << "\n[MAIN] Deteniendo intermediario...\n";
    
    is_running.store(false);
    
    std::string quit_msg = "UPDATE /BEGIN/Q/Ballenitas/0/END/\n";
    std::vector<std::string> all_nodes = network_router->getAllActiveIPs();
    sendAnyway(quit_msg, all_nodes, 8080, true);
    
    if (udp_broadcast_thread.joinable()) udp_broadcast_thread.join();
    if (udp_server_thread.joinable()) udp_server_thread.join();
    if (discovery_thread.joinable()) discovery_thread.join();
    
    delete network_router;
    
    std::cout << "[MAIN] Intermediario detenido\n";
    return 0;
}