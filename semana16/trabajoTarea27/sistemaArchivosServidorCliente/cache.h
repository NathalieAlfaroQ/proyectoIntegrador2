#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <cstdint>
#include <unordered_map>
#include <mutex>

class Cache {
private:
    struct CacheEntry {
        std::string content;
        uint32_t frequency;
    };
    
    std::unordered_map<std::string, CacheEntry> memory;
    uint16_t size;
    uint16_t freeSpace;
    std::mutex cacheMutex;
    
public:
    Cache(uint16_t sizeC);
    
    int add(std::string name, std::string content);
    std::string get(std::string name);
    bool exists(std::string name);
    void invalidate(std::string name);
    int remove(std::string name);
    std::string list();
    uint16_t getFreeSpace();
};

#endif