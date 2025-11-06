#pragma once    
#include <string>
#include <mutex>
#include <unordered_map> 
#include <cstdint>

struct Tuple {
    std::string figure;
    uint frequency;
};

class Cache {
    std::unordered_map<std::string, Tuple> memory;
    std::mutex toControl;
    uint16_t size;
    uint16_t freeSpace;

    public:

    Cache(uint16_t);
    ~Cache();
    int add( std::string , std::string);
    std::string get( std::string );
};