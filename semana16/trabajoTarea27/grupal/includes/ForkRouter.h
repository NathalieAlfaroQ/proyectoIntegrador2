#pragma once

#include <iostream>
#include <semaphore.h>
#include <string>
#include <thread>
#include <vector>

#include "Socket.h"

struct ServerEntry {
   std::string hostname;
   std::string ip;
   std::vector<std::string> figureNames;
};

class ForkRouter {
  public:
    std::string myIP;
    const std::string myHostname = "ballenitas";
    VSocket* currentConnection;
    // Life Cycle
    ForkRouter(std::string myIP);
    ~ForkRouter();

    // Interface
    /// @brief Agrega la información del tenedor a la tabla.
    /// @param hostname El nombre del otro tenedor.
    /// @param ip La dirección IP del otro tenedor.
    /// @param figures Lista de figuras del otro tenedor separadas por cambio de linea (\n).
    bool addFork(const std::string& hostname, const std::string& ip, std::vector<std::string> figures);
    /// @brief Agrega la información del servidor a la tabla.
    /// @param hostname El nombre del servidor.
    /// @param ip La dirección IP del servidor.
    void addServer(const std::string& hostname, const std::string& ip, std::vector<std::string> figures);
    /// @brief Elimina la información del tenedor.
    /// @param ip La dirección IP del otro tenedor.
    void removeFork(const std::string& ip);
    /// @brief Elimina la información del servidor.
    /// @param hostname El nombre del servidor.
    void removeServer();
    /// @brief Agrega la figura a la tabla, si existe el host.
    /// @param hostname El nombre del host a quien asignarle la figura.
    /// @param figureName El nombre de la figura.
    void addFigure(const std::string& hostname, const std::string& figureName);
    /// @brief Eliminar la figura de la tabla, si existe el host.
    /// @param hostname El nombre del host a quien asignarle la figura.
    /// @param figureName El nombre de la figura.
    void deleteFigure(const std::string& hostname, const std::string& figureName);
    /// @brief Busca la IP del host en la tabla del Fork.
    /// @param hostname El nombre del host por buscar.
    /// @return Devuelve la IP del host o un string vacío si no está.
    std::string findHost(const std::string& hostname);
    /// @brief Encuentra la IP del host que contiene la figura.
    /// @param figureName El nombre de la figura por buscar
    /// @return Devuelve la IP del host que contiene la figura. Un string
    /// vacío si no lo encuentra.
    std::string findFigure(const std::string& figureName);\
    /// @brief Indica si el tenedor tiene alguna conexión abierta, de server o tenedor.
    /// @return Devuelve un booleano si hay por lo menos 1 conexión abierta.
    bool isConnected();
    /// @brief Obtiene la lista de figuras del servidor
    /// @return 
    std::string getServerList();
    std::string getEveryoneList();
    std::string getServerIP();
    std::vector<ServerEntry> getForkTable();


  private:
    std::vector<ServerEntry> forkTable;
    ServerEntry figureServer;
    sem_t hasRouter;
    sem_t hasServer;

};