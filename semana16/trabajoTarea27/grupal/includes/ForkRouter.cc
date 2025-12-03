#include <iostream>
#include <semaphore.h>
#include <string>
#include <thread>
#include <vector>

#include "ForkRouter.h"
#include "Socket.h"
#include "ProtocoloGrupal.h"
#include "Utils.h"

// LifeCycle
ForkRouter::ForkRouter(std::string myIP)
        : myIP(myIP) {
  sem_init(&hasRouter, 0, 1);
  sem_init(&hasServer, 0, 0);
}

ForkRouter::~ForkRouter() {
  sem_destroy(&hasRouter);
  sem_destroy(&hasServer);
}

// Interface
bool ForkRouter::addFork(const std::string& hostname, const std::string& ip, std::vector<std::string> figures) {
  ServerEntry entry;
  entry.hostname = hostname;
  entry.ip = ip;
  entry.figureNames = figures;

  // Si existe un host con ese nombre, no se agrega
  if (this->findHost(entry.hostname) != "") return false;

  sem_wait(&hasRouter);

  this->figureServer = entry;

  sem_post(&hasRouter);

  return true;
}

void ForkRouter::addServer(const std::string& hostname, const std::string& ip, std::vector<std::string> figures) {
  ServerEntry entry;
  entry.hostname = hostname;
  entry.ip = ip;
  entry.figureNames = figures;

  sem_wait(&hasRouter);

  this->forkTable.emplace_back(entry);

  sem_post(&hasRouter);

  sem_wait(&hasServer); // Dormir, ya que ya tiene el server

}

void ForkRouter::removeFork(const std::string& ip) {
  bool forkRemoved = false;

  sem_wait(&hasRouter);

  for (auto it = this->forkTable.begin(); it != this->forkTable.end(); it++) {
    if (forkRemoved) {break;}
    if (ip == it->ip) {
      // Borrar figura
      this->forkTable.erase(it);
      forkRemoved = true;
    }
  }

  sem_post(&hasRouter);
}

void ForkRouter::removeServer() {
  sem_wait(&hasRouter);

  this->figureServer = ServerEntry();

  sem_post(&hasRouter);

  sem_post(&hasServer); // Despertar hilo del server

}

void ForkRouter::addFigure(const std::string& hostname, const std::string& figureName) {
  bool figureAdded = false;

  sem_wait(&hasRouter);

  if (this->figureServer.hostname == hostname) {
    this->figureServer.figureNames.emplace_back(figureName);
    figureAdded = true;
  }


  for (struct ServerEntry& serverEntry: this->forkTable) {
    if (figureAdded) {break;}
    if (serverEntry.hostname == hostname) {
      // Borrar figura
      serverEntry.figureNames.emplace_back(figureName);
      figureAdded = true;
    }
  }

  sem_post(&hasRouter);
}

void ForkRouter::deleteFigure(const std::string& hostname, const std::string& figureName) {
  bool figureDeleted = false;

  sem_wait(&hasRouter);

  if (this->figureServer.hostname == hostname) {
    for (auto it = this->figureServer.figureNames.begin(); it != this->figureServer.figureNames.end(); it++) {
      if (figureDeleted) {break;}
      if (figureName == *it) {
        // Borrar figura
        this->figureServer.figureNames.erase(it);
        figureDeleted = true;
      }
    }
  }

  for (struct ServerEntry& serverEntry: this->forkTable) {
    if (figureDeleted) {break;}
    for (auto it = serverEntry.figureNames.begin(); it != serverEntry.figureNames.end(); it++) {
      if (figureDeleted) {break;}
      if (figureName == *it) {
        // Borrar figura
        serverEntry.figureNames.erase(it);
        figureDeleted = true;
      }
    }
  }

  sem_post(&hasRouter);
}

std::string ForkRouter::findFigure(const std::string& figureName) {
  bool figureFound = false;
  std::string IP_Found = "";

  sem_wait(&hasRouter);

  for (const std::string& currentName : this->figureServer.figureNames) {
    if (figureFound) {break;}
    if (figureName == currentName) {
      figureFound = true;
      IP_Found = this->figureServer.ip;
    }
  }

  for (struct ServerEntry& serverEntry: this->forkTable) {
    if (figureFound) {break;}
    for (const std::string& currentName : serverEntry.figureNames) {
      if (figureFound) {break;}
      if (figureName == currentName) {
        figureFound = true;
        IP_Found = this->figureServer.ip;
      }
    }
  }

  sem_post(&hasRouter);

  return IP_Found;
}

std::string ForkRouter::findHost(const std::string& hostname) {
  bool hostFound = false;
  std::string IP_Found = "";

  sem_wait(&hasRouter);

  for (struct ServerEntry& serverEntry: this->forkTable) {
    if (hostFound) {break;}
    if (serverEntry.hostname == hostname) {
      hostFound = true;
      IP_Found = serverEntry.ip;
    }
  }

  sem_post(&hasRouter);

  return IP_Found;
}

bool ForkRouter::isConnected() {
  // Variable para llevar registro
  bool hasA_Connection = false;

  sem_wait(&hasRouter);

  if (this->figureServer.hostname != "") hasA_Connection = true;
  if (this->forkTable.size() != 0) hasA_Connection = true;

  sem_post(&hasRouter);

  return hasA_Connection;
}

std::string ForkRouter::getServerList() {
  std::string figures;

  sem_wait(&hasRouter);

  for (const std::string& figureName : this->figureServer.figureNames) {
    figures.append(figureName + "\n");
  }

  sem_post(&hasRouter);

  return figures;
}

std::string ForkRouter::getEveryoneList() {
  std::string figures;

  figures.append(this->getServerList());

  sem_wait(&hasRouter);

  for (struct ServerEntry& serverEntry: this->forkTable) {
    for (const std::string& currentName : serverEntry.figureNames) {
      figures.append(currentName + "\n");
    }
  }

  sem_post(&hasRouter);

  return figures;
}


std::string ForkRouter::getServerIP() {
  std::string ip;

  sem_wait(&hasRouter);

  ip = this->figureServer.ip;

  sem_post(&hasRouter);

  return ip;
}

std::vector<ServerEntry> ForkRouter::getForkTable() {
  std::vector<ServerEntry> table;

  sem_wait(&hasRouter);

  table = this->forkTable;

  sem_post(&hasRouter);

  return table;
}
