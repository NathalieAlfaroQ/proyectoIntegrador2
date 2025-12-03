#include "Superblock.h"
#include <cmath>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include "Inode.h"
#define inodeSize 42

Superblock::Superblock(std::string diskname, uint32_t sizeDisk, uint32_t blocksize)
    : sizeDisk(sizeDisk), blocksize(blocksize)
{
  disk.open(diskname, std::ios::in | std::ios::out | std::ios::binary);
  if (!disk.is_open()) {
    throw std::runtime_error(diskname);
  }

  allBlocks = sizeDisk / blocksize;

  // Posicionamos al inicio
  disk.seekg(0, std::ios::beg);

  // Leemos freeblocks y usedblocks
  disk.read(reinterpret_cast<char*>(&freeblocks), sizeof(uint16_t));
  disk.read(reinterpret_cast<char*>(&usedblocks), sizeof(uint16_t));

  // Si el disco está recién creado, inicializamos freeblocks
  if (freeblocks == 0 && usedblocks == 0) {
    freeblocks = allBlocks - 1; // reservamos el primer bloque para metadatos
    usedblocks = 0;

    // Inicializamos puntero al primer inode en 0
    uint16_t firstInodeBlock = 0;
    disk.seekp(0, std::ios::beg);
    disk.write(reinterpret_cast<const char*>(&freeblocks), sizeof(uint16_t));
    disk.write(reinterpret_cast<const char*>(&usedblocks), sizeof(uint16_t));
    disk.write(reinterpret_cast<const char*>(&firstInodeBlock), sizeof(uint16_t));
    disk.flush();
  }

  // Leemos puntero al primer inode
  uint16_t next = 0;
  disk.read(reinterpret_cast<char*>(&next), sizeof(uint16_t));

  // Reconstruimos lista de inodes
  while (next != 0) {
    Inode* inode = readInode(next);
    next = inode->next;
    inodes.push_back(inode);
    map[inode->name] = inode;
  }
}

Superblock::~Superblock() {
  // Escribimos freeblocks y usedblocks al inicio
  disk.seekp(0, std::ios::beg);
  disk.write(reinterpret_cast<const char*>(&freeblocks), sizeof(uint16_t));
  disk.write(reinterpret_cast<const char*>(&usedblocks), sizeof(uint16_t));

  // Escribimos el puntero al primer inode
  uint16_t firstInodeBlock = (inodes.empty()) ? 0 : inodes[0]->first_block;
  disk.write(reinterpret_cast<const char*>(&firstInodeBlock), sizeof(uint16_t));

  // Guardamos todos los inodes en el disco
  for (Inode* inode : inodes) {
    writeInode(inode, inode->first_block);
  }

  // Liberamos memoria
  for (Inode* inode : inodes) {
    delete inode;
  }

  inodes.clear();
  map.clear();
}

int Superblock::rm_sb(std::string name) {
  auto it = map.find(name);
  if (it == map.end()) return 1;

  Inode* inode = it->second;

  uint16_t next = inode->first_block;
  for (uint16_t i = 0; i < inode->jump_blocks + 1; i++) {
    if (next == 0) break;
    next = clear_block(next);
  }

  int32_t index = find_inode_by_name(name);

  if (index > 0) {
    if (index + 1 == static_cast<int32_t>(inodes.size())) {
      inodes[index-1]->next = 0;
    } else {
      inodes[index - 1]->next = inodes[index + 1]->first_block;
    }
  }

  inodes.erase(inodes.begin() + index);
  map.erase(name);
  delete inode;

  disk.seekp(sizeof(uint16_t) * 2, std::ios::beg);
  uint16_t firstInodeBlock = (inodes.empty()) ? 0 : inodes[0]->first_block;
  disk.write(reinterpret_cast<const char*>(&firstInodeBlock), sizeof(uint16_t));

  return 0;
}

int Superblock::add_sb(const std::string name, const std::string src) {
  uint16_t first_block = empty_B();
  if (first_block == 0) return 1;

  Inode* inode = new Inode(name, src.size(), first_block, 0);
  const char* cSrc = src.c_str();
  uint32_t remaining = src.size();
  uint16_t next = first_block;
  bool firstIteration = true;

  while (remaining > 0) {
    uint16_t currBlock = next;
    disk.seekp(currBlock * blocksize, std::ios::beg);

    uint32_t toWrite;
    if (firstIteration) {
      writeInode(inode, currBlock);
      toWrite = static_cast<uint32_t>(std::min(remaining,
          static_cast<uint32_t>(blocksize - inodeSize - sizeof(uint16_t))));
      firstIteration = false;
    } else {
      toWrite = static_cast<uint32_t>(std::min(remaining,
          static_cast<uint32_t>(blocksize - sizeof(uint16_t))));
    }

    disk.write(cSrc, toWrite);
    cSrc += toWrite;
    remaining -= toWrite;

    
    next = remaining ? empty_B() : 0;
    inode -> jump_blocks += bool(next);


    disk.seekp(currBlock * blocksize + (blocksize - sizeof(uint16_t)), std::ios::beg);
    disk.write(reinterpret_cast<char*>(&next), sizeof(uint16_t));

    std::cout << "next block: " << next << "\n";

    if (remaining > 0 && next == 0) {
      return 1;
    }
  }

  if (!inodes.empty()) {
    inodes.back()->next = inode->first_block;
  }
  
  inodes.push_back(inode);
  map[inode->name] = inode;
  
  disk.seekp(sizeof(uint16_t) * 2, std::ios::beg);
  uint16_t firstInodeBlock = inodes[0]->first_block;
  disk.write(reinterpret_cast<const char*>(&firstInodeBlock), sizeof(uint16_t));

  return 0;
}

std::string Superblock::get_sb(const std::string name) {
  auto it = map.find(name);
  if (it == map.end() || !it->second) return "";

  Inode* inode = it->second;
  std::string data;
  uint32_t remaining = inode->size;
  
  uint16_t next = inode->first_block;
  bool firstIteration = true;

  while (next != 0 && remaining > 0) {
    disk.seekg(next * blocksize, std::ios::beg);

    uint32_t toRead;
    if (firstIteration) {
      disk.seekg(inodeSize, std::ios::cur);
      toRead = static_cast<uint32_t>(std::min(remaining,
          static_cast<uint32_t>(blocksize - inodeSize - sizeof(uint16_t))));
      firstIteration = false;
    } else {
      toRead = static_cast<uint32_t>(std::min(remaining,
          static_cast<uint32_t>(blocksize - sizeof(uint16_t))));
    }

    char* src = new char[toRead];
    disk.read(src, toRead);
    data.append(src, toRead);
    delete[] src;

    uint16_t next_block = 0;
    disk.seekg(next * blocksize + (blocksize - sizeof(uint16_t)), std::ios::beg);
    disk.read(reinterpret_cast<char*>(&next_block), sizeof(uint16_t));

    std::cout << "next block: " << next_block << "\n";
    next = next_block;
  }

  return data;
}


std::vector<std::string> Superblock::getFiguresNames() {
  
  std::vector<std::string> figuresNames;

  for (Inode* inode : inodes) {
    figuresNames.push_back(inode -> name);
  }

  return figuresNames;
}

uint16_t Superblock::empty_B() {
  for (uint16_t i = 1; i < allBlocks; i++) {
    
    char firstByte = 0;
    disk.seekg(i * blocksize , std::ios::beg);
    disk.read(&firstByte, 1);
    if (!disk) {
      disk.clear();
      continue;
    }

    uint16_t nextField = 0;
    disk.seekg(i * blocksize + (blocksize - sizeof(uint16_t)), std::ios::beg);
    disk.read(reinterpret_cast<char*>(&nextField), sizeof(uint16_t));
    if (!disk) {
      disk.clear();
      continue;
    }

    if (firstByte == 0 && nextField == 0) {
      usedblocks++;
      if (freeblocks > 0) freeblocks--;
      return i;
    }
  }

  return 0;
}


uint16_t Superblock::clear_block(uint16_t block_index) {
  if (block_index == 0 || block_index >= allBlocks) return 0;

  uint16_t next = 0;
  disk.seekg(block_index * blocksize + blocksize - sizeof(uint16_t), std::ios::beg);
  disk.read(reinterpret_cast<char*>(&next), sizeof(uint16_t));

  char* zero_block = new char[blocksize]();
  disk.seekp(block_index * blocksize, std::ios::beg);
  disk.write(zero_block, blocksize);

  if (usedblocks > 0) usedblocks--;
  if (freeblocks < allBlocks) freeblocks++;

  return next;
}

int32_t Superblock::find_inode_by_name(std::string name) {
  for (uint32_t i = 0; i < this->inodes.size(); ++i) {
    inodes[i] -> name;
    if (inodes[i] -> name == name) {
      return static_cast<int32_t>(i);
    }  
  }
  return -1;
}

Inode* Superblock::readInode(uint16_t block_index){
  char* blockBuffer = new char[blocksize]();
  disk.seekg(block_index * blocksize, std::ios::beg);
  disk.read(blockBuffer, blocksize);

  Inode* inode = new Inode();
  std::strncpy(inode->name, blockBuffer, maxSize);
  inode->size = *reinterpret_cast<uint32_t*>(blockBuffer + maxSize);
  inode->first_block = *reinterpret_cast<uint16_t*>(blockBuffer + maxSize + sizeof(uint32_t));
  inode->jump_blocks = *reinterpret_cast<uint16_t*>(blockBuffer + maxSize + sizeof(uint32_t) + sizeof(uint16_t));
  inode->next = *reinterpret_cast<uint16_t*>(blockBuffer + maxSize + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t));

  return inode;
}

int Superblock::writeInode(Inode* inode, uint16_t block_index){
  disk.seekp(block_index * blocksize, std::ios::beg);

  disk.write(reinterpret_cast<char*>(inode->name), maxSize);
  disk.write(reinterpret_cast<char*>(&(inode->size)), sizeof(uint32_t));
  disk.write(reinterpret_cast<char*>(&(inode->first_block)), sizeof(uint16_t));
  disk.write(reinterpret_cast<char*>(&(inode->jump_blocks)), sizeof(uint16_t));
  disk.write(reinterpret_cast<char*>(&(inode->next)), sizeof(uint16_t));

  return 0;
}