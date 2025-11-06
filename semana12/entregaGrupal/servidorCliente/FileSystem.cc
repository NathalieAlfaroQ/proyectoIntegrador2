#include <filesystem>
#include <iostream>
#include <algorithm>

#include "FileSystem.h"
#include "Superblock.h"

#define blocksize 256
#define sizeDisk (256 * 26)
#define maxSize 32

FileSystem::FileSystem(): numDisk(0) {
  std::vector<std::string> files = lookForDisk(".");

  for (const auto& i : files) {
    Superblock* sb = new Superblock(i, sizeDisk, blocksize);
    bloques.push_back(sb);
    numDisk++;  

    for (const auto& s : sb->getFiguresNames()) {
      map[s] = sb;
    }
  }
}

FileSystem::~FileSystem() {
  for (auto sb : bloques)
    delete sb;
}

int FileSystem::rm(std::string name) {
  std::unique_lock<std::mutex> generalLock(knowSuperblock);
  auto it = map.find(name);
  if (it == map.end()) return 1;

  Superblock* sb = it->second;
  map.erase(name); 

  {
    std::lock_guard<std::mutex> guard2(sb->diskMutex);
    generalLock.unlock();
    int res = sb->rm_sb(name);
    return res;
  }
}

int FileSystem::add(std::string name, std::string draw) {
  std::unique_lock<std::mutex> generalLock(knowSuperblock);
  name = name.substr(0, maxSize);
  auto it = map.find(name);
  if (it != map.end()) return 1;
  if (draw.size() > sizeDisk - blocksize) return 1;


  Superblock* sb = nullptr;

  auto that = std::max_element(
    bloques.begin(), bloques.end(),
    [](const Superblock* a, const Superblock* b) {
      return a->freeblocks < b->freeblocks;
    }
  );

  if (that != bloques.end() && (*that)->freeblocks * blocksize >= draw.size()) {
    sb = *that;
  } else {
    sb = new Superblock(createDisk(), sizeDisk, blocksize);
    bloques.push_back(sb);
  }
  
  map[name] = sb;

  {
    std::lock_guard<std::mutex> guard2(sb->diskMutex);
    int i = sb->add_sb(name, draw);
    generalLock.unlock();
    return i;
  }
}


std::string FileSystem::get(std::string name) {
  std::unique_lock<std::mutex> generalLock(knowSuperblock);
  auto it = map.find(name);
  if (it == map.end()) return "";

  Superblock* sb = it->second;
  {
    std::lock_guard<std::mutex> guard2(sb->diskMutex);
    generalLock.unlock();
    return sb->get_sb(name);
  }
}


std::string FileSystem::list(){
  std::unique_lock<std::mutex> generalLock(knowSuperblock);
  std::string list = "\n";

  for (const auto& i : map) {
    list += i.first + "\n";
  }

  return list;
}

std::vector<std::string> FileSystem::lookForDisk(std::string path) {
  std::vector<std::string> files;

  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    std::string archiveName = entry.path().filename().string();
    if (archiveName.find("disk") != std::string::npos) {
      files.push_back(archiveName);
    }
  }

  return files;
}

std::string FileSystem::createDisk() {
  std::string name = "disk";
  name += std::to_string(numDisk);
  name += ".bin";

  std::ofstream file(name, std::ios::binary | std::ios::trunc);
  char zero[blocksize] = {0};
  for (size_t i = 0; i < sizeDisk / blocksize; ++i) {
    file.write(zero, blocksize);
  }
  
  file.close();

  numDisk++;

  return name;
}