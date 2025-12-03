#pragma once
#include <string>
#include <fstream>
#include <mutex> 
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Superblock.h"

class FileSystem
{
private:
    std::unordered_map<std::string, Superblock*> map;
    std::vector<Superblock*> bloques;
    std::mutex knowSuperblock;
    int numDisk;

    std::vector<std::string> lookForDisk(std::string path);
    std::string createDisk();
    Superblock* getSuperblock();
    
public:
    FileSystem();
    ~FileSystem();
    int rm( std::string );
    int add( std::string , std::string);
    std::string get( std::string );
    std::string list();
};