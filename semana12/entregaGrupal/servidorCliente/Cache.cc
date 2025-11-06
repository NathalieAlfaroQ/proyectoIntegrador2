#include "Cache.h"
#include <string>
#include <mutex>
#include <unordered_map>
#include <cstdint> 
#include <climits>

Cache::Cache(uint16_t sizeC): size(sizeC), freeSpace(sizeC) {

}

Cache::~Cache(){
}

int Cache::add( std::string name , std::string content){
    std::lock_guard<std::mutex> generalLock(toControl);

    if (content.size() > size) {
        memory.clear();
        freeSpace = size;
        return 0;
    }

    while (freeSpace < content.size() && !memory.empty()) {
        std::string toRemove;
        uint lastFrequency = UINT_MAX;

        for (auto& [key, value] : memory) {
            if (value.frequency < lastFrequency) {
                toRemove = key;
                lastFrequency = value.frequency;
            }
        }

        freeSpace += memory[toRemove].figure.size();
        memory.erase(toRemove);
    }

    memory[name] = Tuple{content, 1};
    freeSpace -= content.size();
    return 0;
}

std::string Cache::get( std::string name ){
  std::lock_guard<std::mutex> generalLock(toControl);
  auto it = memory.find(name);
  if (it == memory.end()) return "";

  memory[name].frequency++;
  return memory[name].figure;
}
