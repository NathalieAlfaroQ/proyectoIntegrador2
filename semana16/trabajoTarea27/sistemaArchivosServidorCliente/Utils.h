#pragma once

#include <string>
#include "VSocket.h"
#include "Socket.h"
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <vector>


std::string getLocalIP();

std::string getInterfaceName();

std::string getBroadcastIP(const std::string& iface = "wlp2s0");

bool waitMessage(Socket& s, int timeoutSec, std::string& outMsg, sockaddr_in& outSender);

bool handshake(std::string& ipExServer, std::string hostname, int port);

int sendAnyway(std::string message, std::vector<std::string> ips, int port, bool tcp);

int readProto(VSocket& sock, std::string& headers);

int readHTTP(VSocket& sock, std::string& headers);

bool readVerify(VSocket& sock, std::string& data);

int hexValue(char c);

std::string urlDecode(const std::string& s);

std::vector<std::string> split(const std::string& text, const std::string& pattern);

std::string buildHtml(const std::string& proto);

std::string renderHTML(const std::string& proto);

std::string rRequestHTTP(const std::string& proto, bool list);