#include <arpa/inet.h>     
#include <sys/socket.h>   
#include <sys/types.h>    
#include <netinet/in.h>     
#include <net/if.h>      
#include <sys/ioctl.h>  
#include <ifaddrs.h>
#include <unistd.h>         
#include <sys/select.h>     
#include <netdb.h>          
#include <string>
#include <cstring>
#include <stdexcept>
#include <regex>
#include <iostream>
#include <fstream>
#include <vector>
#include "VSocket.h"
#include "Socket.h"
#include "Utils.h"

std::string getLocalIP(const std::string& iface) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return "0.0.0.0";

    struct ifreq ifr {};
    std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);

    // Pedir dirección IP asignada a la interfaz
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
        close(fd);
        return "0.0.0.0";
    }

    close(fd);

    struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
    return std::string(inet_ntoa(ipaddr->sin_addr));
}

std::string getInterfaceName() {
    struct ifaddrs* ifaddr;

    if (getifaddrs(&ifaddr) == -1)
        return "";

    std::string interfaz = "";

    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {

            if (ifa->ifa_flags & IFF_LOOPBACK) continue;

            interfaz = ifa->ifa_name;
            break;
        }
    }

    freeifaddrs(ifaddr);
    return interfaz;
}

std::string getBroadcastIP(const std::string& iface) {
    Socket s('d');
    struct ifreq ifr{};
    std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);

    if (ioctl(s.returnFd(), SIOCGIFBRDADDR, &ifr) < 0) {
        throw std::runtime_error("No se pudo obtener la dirección de broadcast");
    }

    struct sockaddr_in* br = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_broadaddr);
    char buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &br->sin_addr, buffer, sizeof(buffer));

    return std::string(buffer);
}

bool waitMessage(Socket& s, int timeoutSec, std::string& outMsg, sockaddr_in& outSender, std::string interface) {

    std::cout << "[waitMessage] Esperando mensajes en fd=" << s.returnFd() 
              << " timeout=" << timeoutSec << "\n";

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(s.returnFd(), &fds);

    timeval tv{};
    tv.tv_sec = timeoutSec;
    tv.tv_usec = 0;

    int ret = select(s.returnFd() + 1, &fds, nullptr, nullptr, &tv);

    std::cout << "[waitMessage] Select retornó: " << ret << "\n";

    if (ret <= 0) {
        std::cout << "[waitMessage] Timeout o error en select\n";
        return false;
    }

    sockaddr_in self{};
    self.sin_family = AF_INET;
    self.sin_port = htons(s.returnPort());
    inet_aton(getLocalIP(interface).c_str(), &self.sin_addr);

    std::cout << "[waitMessage] Ignorando mensajes de IP=" << getLocalIP(interface)
              << " PORT=" << s.returnPort() << "\n";

    char buffer[2048];

    while (true) {
        std::cout << "[waitMessage] Llamando a recvFrom...\n";

        ssize_t n = s.recvFrom(buffer, sizeof(buffer)-1, &outSender);

        if (n <= 0) {
            std::cout << "[waitMessage] recvFrom devolvió <=0, error\n";
            return false;
        }

        char senderIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &outSender.sin_addr, senderIP, sizeof(senderIP));
        int senderPort = ntohs(outSender.sin_port);

        std::cout << "[waitMessage] Mensaje recibido de " 
                  << senderIP << ":" << senderPort << "\n";

        if (outSender.sin_addr.s_addr == self.sin_addr.s_addr &&
            outSender.sin_port == self.sin_port) {

            std::cout << "[waitMessage] Ignorado: mensaje es de nosotros mismos\n";
            continue;
        }

        buffer[n] = '\0';
        outMsg = buffer;

        std::cout << "[waitMessage] Mensaje aceptado: " << outMsg << "\n";

        return true;
    }
}

bool handshake(std::string& ipExServer, std::string hostname, int port, std::string interface) {

    std::string localIP = getLocalIP(interface);
    std::string iface   = interface;
    std::string bcast   = getBroadcastIP(iface);

    std::cout << "[handshake] IP local: " << localIP << "\n";
    std::cout << "[handshake] Interfaz: " << iface << "\n";
    std::cout << "[handshake] Broadcast: " << bcast << "\n";
    std::cout << "[handshake] Puerto local: " << port << "\n";

    try {
        Socket s('d');
        std::cout << "[handshake] Creado socket UDP\n";

        s.Bind(port);
        std::cout << "[handshake] Bind exitoso en puerto " << port << "\n";

        int opt = 1;
        setsockopt(s.returnFd(), SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
        std::cout << "[handshake] SO_BROADCAST habilitado\n";

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        addr.sin_addr.s_addr = inet_addr(bcast.c_str());

        while (true) {

            std::string msg = "CONNECT /BEGIN/" + localIP + "/" + hostname + "/END/\n";
            std::cout << "[handshake] Enviando broadcast: " << msg << "\n";

            s.sendTo(msg.c_str(), msg.size(), &addr);

            std::cout << "[handshake] Esperando respuesta...\n";

            std::string received;
            sockaddr_in sender{};

            if(!waitMessage(s, 10, received, sender)) {
                std::cout << "[handshake] Timeout, reenviando handshake...\n";
                continue;
            }

            // Info del remitente
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &sender.sin_addr, ipStr, sizeof(ipStr));
            std::string ipSender = ipStr;

            std::cout << "[handshake] Respuesta recibida de " << ipSender << "\n";
            std::cout << "[handshake] Contenido: " << received << "\n";

            std::regex re1("CONNECT\\s/BEGIN/([\\d\\.]+)/([\\w\\.-]+)/END/\n");
            std::regex re2("/BEGIN/200/END/\n");

            std::smatch match;

            if (std::regex_search(received, match, re1)) {
                std::cout << "[handshake] Petición de cliente detectada\n";

                std::string fmsg = "/BEGIN/200/END/\n";
                s.sendTo(fmsg.c_str(), fmsg.size(), &sender);

                ipExServer = ipSender;

                std::cout << "[handshake] Respondido OK al cliente\n";
                return true;
            }

            if (std::regex_search(received, match, re2)) {
                std::cout << "[handshake] Confirmación 200 OK detectada\n";

                ipExServer = ipSender;
                return true;
            }

            std::cout << "[handshake] Mensaje desconocido, ignorado\n";
        }

    } catch (std::exception& e) {

        std::cout << "[handshake] EXCEPCIÓN: " << e.what() << "\n";
        std::cout << "[handshake] Modo fallback (respuesta de emergencia)\n";

        Socket s('d');

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        addr.sin_addr.s_addr = inet_addr(localIP.c_str());

        std::string fmsg = "/BEGIN/200/END/\n";
        s.sendTo(fmsg.c_str(), fmsg.size(), &addr);

        ipExServer = localIP;

        std::cout << "[handshake] Enviado 200 OK de emergencia a " << localIP << "\n";

        return true;
    }
}

int sendAnyway(std::string message, std::vector<std::string> ips, int port, bool tcp) {
    if (tcp) {
        for (const auto& addr : ips) {
            try {
                Socket s('s');  
                s.MakeConnection(addr.c_str(), port);
                s.Write(message.c_str(), message.size());
            } catch (const std::exception& e) {
                std::cerr << "[TCP] Error enviando a " << addr << ": " << e.what() << "\n";
            }
        }
    } else {
        for (const auto& addr : ips) {
            try {
                Socket s('d');
                struct sockaddr_in dest;
                memset(&dest, 0, sizeof(dest));
                dest.sin_family = AF_INET;
                dest.sin_port   = htons(port);

                if (inet_pton(AF_INET, addr.c_str(), &dest.sin_addr) <= 0) {
                    throw std::runtime_error("Invalid broadcast address: " + addr);
                }

                int opt = 1;
                setsockopt(s.returnFd(), SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

                s.sendTo(message.c_str(), message.size(), &dest);

            } catch (const std::exception& e) {
                std::cerr << "[UDP] Error enviando a " << addr << ": " << e.what() << "\n";
            }
        }
    }

    return 0;
}

int readHTTP(VSocket& sock, std::string& headers) {

    size_t contentLength = 0;

    
    {
        size_t clPos = headers.find("Content-Length:");
        if (clPos != std::string::npos) {
            size_t endPos = headers.find("\r\n", clPos);
            std::string value = headers.substr(clPos + 15, endPos - (clPos + 15));
            contentLength = std::stoul(value);
        }
    }

    std::string body;
    if (contentLength > 0) {
        body.resize(contentLength);
        size_t totalRead = 0;

        while (totalRead < contentLength) {
            ssize_t n = sock.Read(&body[totalRead], contentLength - totalRead);
            if (n <= 0) throw std::runtime_error("Conexión cerrada");
            totalRead += n;
        }

        headers += body;
    }

    std::istringstream req(headers);
    std::string method, path, version;
    req >> method >> path >> version;
    
    if (method == "POST" && path == "/ADD%20") {
        
        if (body.rfind("combinado=", 0) == 0) {
            std::string value = body.substr(body.find("combinado=") + 10);

            
            std::regex re("ADD\\s/BEGIN/([\\w.]+)/END/\r?\n([\\s\\S]*)");
            std::smatch m;

            if (std::regex_search(value, m, re)) {
                std::string nombre = m[1];
                std::string contenido = m[2];

                
                headers =
                    "ADD /BEGIN/" + nombre + "/" +
                    std::to_string(contenido.size()) +
                    "/END/\n" + contenido;

                return 0;
            }
        }
    }

    headers = urlDecode(path);
    headers = headers.substr(1);
    
    std::string token = "/END/";
    size_t pos = headers.find(token);

    if (pos != std::string::npos) {
        headers.insert(pos + token.size(), "\n");
    }

    return 0;
}


int readProto(VSocket& sock, std::string& headers) {

    int verify = 0;
    verify += headers.find("LIST") != std::string::npos;
    verify += headers.find("DELETE") != std::string::npos;
    verify += headers.find("GET") != std::string::npos;

    if (verify > 0) {
        return 0;
    }

    size_t contentLength = 0;

    if (headers.find("ADD") != std::string::npos) {
        std::regex reAdd("ADD\\s/BEGIN/[\\w\\.]+/([\\d]+)/END/\n");
        std::smatch match;
        if (std::regex_search(headers, match, reAdd)) {
            contentLength = std::stoul(match[1]);
        }

    } else if (headers.find("UPDATE") != std::string::npos){
        std::regex re(R"(UPDATE\s/BEGIN/(.)/([\w\.-]+)/(\d+)/END/\n)");
        std::smatch match;

        if (std::regex_search(headers, match, re)) {
            contentLength = std::stoul(match[3]);
        }
    
    } else {
        std::regex reResp("/BEGIN/\\d{3}(?:/(\\d+))?/END/\n");
        std::smatch match;
        if (std::regex_search(headers, match, reResp)) {
            if (match.size() >= 2 && match[1].matched) {
                contentLength = std::stoul(match[1]);
            }
        }
    }

    if (contentLength > 0) {
        std::string body(contentLength, '\0');
        size_t totalRead = 0;

        while (totalRead < contentLength) {
            ssize_t n = sock.Read(&body[totalRead], contentLength - totalRead);
            totalRead += n;
        }
        headers += body;
    }

    return 0;
}

bool readVerify(VSocket& sock, std::string& data) {
    char c;
    bool protocol = true;

    while (true) {
        ssize_t n = sock.Read(&c, 1);
        if (n <= 0) {
            throw std::runtime_error("Conexión cerrada antes de completar encabezado");
        }
        data.push_back(c);

        if (data.size() >= 6 &&
            data.substr(data.size() - 6) == "/END/\n") {
            break;
        }

        if (data.size() >= 4 &&
            data.find("HTTP/") != std::string::npos &&
            data.substr(data.size() - 4) == "\r\n\r\n") {
            protocol = false;
            break;
        }
    }

    if (protocol) {
        readProto(sock, data);
    } else {
        readHTTP(sock, data);
    }

    return protocol;
}

int hexValue(char c) {
    if (std::isdigit(c)) return c - '0';
    return std::tolower(c) - 'a' + 10;
}

std::string urlDecode(const std::string& s) {
    std::string out;
    out.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size() &&
            std::isxdigit(s[i+1]) && std::isxdigit(s[i+2])) {

            int value = (hexValue(s[i+1]) << 4) | hexValue(s[i+2]);
            out.push_back(static_cast<char>(value));
            i += 2;
        }
        else if (s[i] == '+') {
            out.push_back(' ');
        }
        else {
            out.push_back(s[i]);
        }
    }

    return out;
}

std::vector<std::string> split(const std::string& text, const std::string& pattern){
  std::regex spliter(pattern);
  std::sregex_token_iterator iter(text.begin(), text.end(), spliter, -1);
  std::sregex_token_iterator end;

  std::vector<std::string> result;
  while (iter != end) {
    if (!iter->str().empty()) {
      result.push_back(*iter);
    } 
    ++iter;
  }
  return result;
}

std::string buildHtml(const std::string& proto) {
    std::string html;
    std::smatch m;

    std::regex re1("/BEGIN/(\\d{3})/(\\d+)/END/\n([\\s\\S]*)");
    if (std::regex_search(proto, m, re1)) {

        std::vector<std::string> figuras = split(m[3], "\n");

        html += "<!DOCTYPE html>\n<html>\n<head>\n";
        html += "<meta charset=\"UTF-8\">\n<title>Figuras</title>\n";

        html += R"(
            <style>
            ul { list-style:none; padding:0; margin:0 0 20px 0; }
            li.entry { display:flex; align-items:center; gap:10px; padding:6px 0; }
            a.figure-link { text-decoration:none; color:#0366d6; font-family:monospace; }
            form.inline { display:inline; margin:0; padding:0; }
            button.delete-btn { margin-left:8px; }
            .add-form { margin-top:20px; }
            label { display:block; margin-bottom:6px; font-weight:600; }
            </style>

            <script>
            function sendAdd() {
                const nombre = document.getElementById('nombre').value.trim();
                const contenido = document.getElementById('contenido').value;

                if (!nombre || !contenido) {
                    alert('Debe ingresar nombre y contenido.');
                    return;
                }

                // Construimos la solicitud EXACTA para el servidor
                const combinado =
                    "ADD /BEGIN/" + nombre + "/END/\n" + contenido;

                // Insertamos en el formulario oculto
                document.getElementById('combinado').value = combinado;

                // Enviamos el formulario oculto
                document.getElementById('form_add').submit();
            }
            </script>
            )";

        html += "</head>\n<body>\n";
        html += "<h2>Seleccione una figura</h2>\n<ul>\n";

        for (const auto& fig : figuras) {
            html += "<li class=\"entry\">";
            html += "<a class=\"figure-link\" href=\"/GET%20/BEGIN/" + fig + "/END/\">" +
                    fig + "</a>";

            html += "<form class=\"inline\" method=\"GET\" "
                    "action=\"/DELETE%20/BEGIN/" + fig + "/END/\">\n";
            html += "<input type=\"hidden\" name=\"figura\" value=\"" + fig + "\">\n";
            html += "<button class=\"delete-btn\" type=\"submit\">Eliminar</button>\n";
            html += "</form></li>\n";
        }

        html += "</ul>\n";

        // FORMULARIO ADD
        html += "<div class=\"add-form\">\n";
        html += "<h3>Agregar nueva figura</h3>\n";

        // Campos visibles fuera del formulario
        html += "<label for=\"nombre\">Nombre:</label>\n";
        html += "<input type=\"text\" id=\"nombre\" required>\n";

        html += "<label for=\"contenido\">Contenido:</label>\n";
        html += "<textarea id=\"contenido\" rows=\"10\" cols=\"80\" required></textarea>\n";

        // Formulario oculto real
        html += "<form id=\"form_add\" method=\"POST\" action=\"/ADD%20\" enctype=\"text/plain\" style=\"display:none;\">\n";
        html += "<input type=\"hidden\" id=\"combinado\" name=\"combinado\">\n";
        html += "</form>\n";

        // Botón que llama JS
        html += "<br><br><button onclick=\"sendAdd()\">Agregar</button>\n";

        html += "</div>\n</body>\n</html>\n";
    }

    return html;
}

std::string renderHTML(const std::string& proto) {
    std::string html;
    std::smatch m;

    std::regex re2("/BEGIN/(\\d{3})/(\\d+)/END/\n([\\s\\S]*)");
    if (std::regex_search(proto, m, re2)) {
        std::string code   = m[1];
        std::string figura = "\n" + m[3].str();

        html = R"(
            <html>
            <head><meta charset="UTF-8"><title>Figura</title></head>
            <h2>Figura</h2>
            <pre style="font-size:14px; white-space:pre;">
            )" + figura + R"(</pre>
            </body>
            </html>
        )";
    }

    std::regex re3("/BEGIN/(\\d{3})/END/");
    if (std::regex_search(proto, m, re3)) {
        std::string code = m[1];

        return R"(
            <html>
            <head><meta charset="UTF-8"><title>Mensaje</title></head>
            <body>
            <h2>Respuesta</h2>
            <p>Código: )" + code + R"(</p>
            </body>
            </html>
        )";
    }

    return html;
}

std::string rRequestHTTP(const std::string& proto, bool list) {
    std::string html = list? buildHtml(proto) : renderHTML(proto);
    std::string response = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Content-Length: " + std::to_string(html.size()) + "\r\n"
    "\r\n" +
    html;

    return response;
}
