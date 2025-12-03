#include <stdio.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <regex>
#include <csignal>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <atomic>
#include "../includes/VSocket.h"
#include "../includes/Socket.h"
#include "../includes/FileSystem.h"
#include "../includes/Utils.h"

FileSystem* fs = nullptr;

std::string direccionConexion;
std::string hostname;

std::mutex handshake_mutex;
std::atomic<bool> cache_is_recovering(false);
std::atomic<bool> cache_is_online(true);

std::atomic<bool> end(false);

void handle_sigint(int) {
    end.store(true);
}

void hilo_atencion(VSocket* client) {
    try {
        bool error = false;
        std::string request;
        readVerify(*client, request);

        std::cout << "[DEBUG] Solicitud:\n" << request << "\n";

        std::string response;

        bool isLIST   = request.rfind("LIST",   0) == 0;
        bool isDELETE = request.rfind("DELETE", 0) == 0;
        bool isGET    = request.rfind("GET",    0) == 0;
        bool isADD    = request.rfind("ADD",    0) == 0;

        if (isLIST) {

            std::string listStr = fs->list();

            response = "/BEGIN/200/" + std::to_string(listStr.size()) +
                       "/END/\n" + listStr;

            std::cout << "[DEBUG] Respuesta LIST enviada.\n";
        }
        else if (isDELETE) {

            std::regex re("DELETE\\s/BEGIN/([\\w\\.]+)/");
            std::smatch match;

            if (std::regex_search(request, match, re)) {
                std::string filename = match[1];

                try {
                    if (!cache_is_online.load()) {
                        
                        int r = fs->rm(filename);
                        response = r == 0 ?
                            "/BEGIN/200/END/\n" :
                            "/BEGIN/404/END/\n";
                    } else {
                        if (direccionConexion.empty())
                            throw std::runtime_error("cache no disponible");

                        Socket cache('s');
                        cache.MakeConnection(direccionConexion.c_str(), 8082);
                        cache.Write(request.c_str(), request.size());

                        std::string answerCache;
                        readVerify(cache, answerCache);

                        if (answerCache.find("404") != std::string::npos) {
                            int r = fs->rm(filename);
                            response = r == 0 ?
                                "/BEGIN/200/END/\n" :
                                "/BEGIN/404/END/\n";
                        } else {
                            response = answerCache;
                        }
                    }
                } catch (std::exception& e) {
                    
                    int r = fs->rm(filename);
                    response = r == 0 ?
                        "/BEGIN/200/END/\n" :
                        "/BEGIN/404/END/\n";

                    error = true;
                }
            } else {
                response = "/BEGIN/400/END/\n";
            }
        }
        else if (isGET) {

            std::regex re("GET\\s/BEGIN/([\\w\\.]+)/");
            std::smatch match;

            if (std::regex_search(request, match, re)) {

                std::string filename = match[1];
                std::string content;

                try {
                    if (!cache_is_online.load()) {
                        content = fs->get(filename);

                        if (!content.empty()) {
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
                            content = fs->get(filename);
                            if (!content.empty()) {
                                response =
                                    "/BEGIN/200/" + std::to_string(content.size()) +
                                    "/END/\n" + content;

                                std::string addReq =
                                    "ADD /BEGIN/" + filename + "/" +
                                    std::to_string(content.size()) +
                                    "/END/\n" + content;

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
                    content = fs->get(filename);

                    if (!content.empty()) {
                        response =
                            "/BEGIN/200/" + std::to_string(content.size()) +
                            "/END/\n" + content;
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
                fs->add(match[1].str(), match[2].str());
                response = "/BEGIN/200/END/\n";
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
                bool ok = handshake(direccionConexion, hostname, 5687, getInterfaceName());

                cache_is_online.store(ok);
            }

            cache_is_recovering.store(false);
        }

    } catch (const std::exception& e) {
        std::cout << "[ERROR] Socket exception: " << e.what() << "\n";
    }

    delete client;
    std::cout << "[DEBUG] Conexión cerrada.\n";
}


int main(int argc, char** argv) {
    
    std::signal(SIGINT, handle_sigint);
    std::signal(SIGPIPE, SIG_IGN);

    std::string hostname = argc < 2 ? "figure5" : argv[1];
    std::string tenedorIP;

    //handshake(direccionConexion, hostname, 5687);
    handshake(tenedorIP, hostname, 1234, getInterfaceName());

    fs = new FileSystem();
 
    int port = 8081;

    Socket server('s');          
    server.Bind(port);
    server.MarkPassive(5);

    std::cout << "Servidor activo en puerto " << port << "\n";

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

    Socket respond('d');

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(1234);
    addr.sin_addr.s_addr = inet_addr(tenedorIP.c_str());

    std::string msg = "QUIT /BEGIN/" + getLocalIP(getInterfaceName()) + "/END/\n";

    //respond.sendTo(msg.c_str(), msg.size(), &addr);
    
    std::cout << "Servidor detenido con Ctrl+C, guardando datos...\n";
    delete fs;

    return 0;
}