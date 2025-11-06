#pragma once
#include <string>
#include <cstdint>
#define maxSize 32

struct Inode {
    char name[maxSize];
    uint32_t size;
    uint16_t first_block;
    uint16_t jump_blocks;
    uint16_t next = 0;

  Inode(){
  }

  Inode(std::string name, uint32_t s, uint16_t fb, uint16_t jb){    
    
    if(name.size() > maxSize){
      name = name.substr(0, maxSize); 
    }

    snprintf(this -> name, maxSize, "%s", name.c_str());

    size = s;
    first_block = fb;
    jump_blocks = jb;
  }
};