#pragma once
#include <unordered_map>
#include <vector>
#include <mutex>
#include <fstream>
#include "Inode.h"

class Superblock {

  public:

  std::unordered_map<std::string, Inode*> map;
  std::vector<Inode*> inodes;
  std::fstream disk;
  std::mutex diskMutex;
  uint32_t sizeDisk;
  uint32_t blocksize;
  uint16_t allBlocks;
  uint16_t freeblocks;
  uint16_t usedblocks;

  Superblock( std::string, uint32_t sizeDisk, uint32_t blocksize);
  ~Superblock();
  
  int rm_sb( std::string );
  int add_sb( std::string , std::string);
  std::string get_sb( std::string);
  std::vector<std::string> getFiguresNames();

  private:

  uint16_t empty_B();
  uint16_t clear_block(uint16_t);
  int32_t find_inode_by_name(std::string);
  Inode* readInode(uint16_t);
  int writeInode(Inode*, uint16_t);

};