#include <iostream>
#include <utility>
#include <algorithm>
#include <filesystem>

#include "FileSystem.hpp"
#include "BlockOper.hpp"

constexpr const size_t BLOCK_SIZE = 256; // Cantidad de bloques

// LIFECYCLE
FileSystem::FileSystem(const std::string& fileName)
        : disk(fileName) { // TODO(): Is this line using the propper constructor for File Handler?
  if (this->loadDirectory() != 0) {
    throw std::runtime_error("Directory::Failed to open file");
  }
}

FileSystem::~FileSystem() {
}

// CORE FUNCTIONALITY
std::string FileSystem::getDirectory() {
  if (this->directory.empty()) return "No files available\n";

  // Append dir info to a string (format) and send in return.
  std::string directoryData;
  for (const std::pair<std::string, uint16_t>& fileLog : this->directory) {
    directoryData += fileLog.first + "\n";
  }
  return directoryData;
}


std::string FileSystem::getDirectoryHTML() {
  if (this->directory.empty()) return "<h1>No files available :(</h1>";

  // Append dir info to a string (format like an unordered list) and send in return.
  std::string directoryData;
  directoryData += "<ul>\n";
  for (const std::pair<std::string, uint16_t>& fileLog : this->directory) {
    directoryData += "<li>" + fileLog.first +  "</li>\n";
  }
  directoryData += "</ul>\n";
  return directoryData;
}


std::string FileSystem::getFile(std::string fileName) {
  // If no block table found, return a not found
  uint16_t blockTable = this->getFileBlockTable(fileName);
  if (blockTable == 0) return "";

  std::string fileData;
  std::vector<unsigned char> blockTableBuffer(256);
  std::vector<unsigned char> blockBuffer(256);

  // Obtain data from the file
  while (blockTable != 0) {
    // Load block table's content
    if (this->disk.readBlock(blockTable, blockTableBuffer) != 0) return "Error reading block";
    // read 2 bytes. If 0, stop. Else, read block and load into fileData.
    for (uint16_t currentByte = 0; currentByte < 256; currentByte+=2) {
      // Get 2 bytes for the pointer, using little endian.
      uint16_t blockPointer = read2BPointer(blockTableBuffer, currentByte);
      // End of table, pointer to null block
      if (blockPointer == 0) {
        return fileData;
      // End of block, get new block table
      } else if (currentByte == 254) {
        blockTable = blockPointer;
        break;
      } else {
        // Obtain data in new block
        if (this->disk.readBlock(blockPointer, blockBuffer)) return "Error reading block";
        fileData.append(reinterpret_cast<const char*>(blockBuffer.data()), 256);
      }
    }
  }
  return fileData;
}


int FileSystem::addFile(std::string name, std::istream& fileContent) {
  // Allocate block for the first block table
  uint16_t currentTableBlock = this->disk.allocateBlock();
  if (currentTableBlock == 0) {
      std::cerr << "Error: No space for initial block table\n";
      return -1;
  }

  // Add to the directory
  if (this->getFileBlockTable(name) != 0 ) {
    printf("Error: duplicated name for a file.\n");
    return -1;
  }
  this->directory.emplace_back(std::move(name), currentTableBlock);
  if (this->updateDir() != 0) return -1;

  // ! TODO(jesus): hay que corregir esto para que no se desborde con archivos.
  // if(!this->disk.hasStorageFor(std::filesystem::file_size(name))) {
  //   printf("Error: not enough storage for the file.\n");
  //   return -1;
  // }

  std::vector<unsigned char> blockTable(BLOCK_SIZE, 0);
  uint16_t currentEntry = 0;
  uint16_t dataBlockCount = 0;
  bool needsFinalWrite = false;

  try {
      std::vector<unsigned char> buffer(BLOCK_SIZE);
      while (fileContent.read(reinterpret_cast<char*>(buffer.data()), BLOCK_SIZE) || fileContent.gcount() > 0) {
          // Allocate and write data block
          uint16_t dataBlock = this->disk.allocateBlock();
          if (dataBlock == 0) throw std::runtime_error("No space for data block");

          // Store data block pointer in current block table
          write2BPointer(blockTable, currentEntry * 2, dataBlock);
          currentEntry++;
          dataBlockCount++;

          // Write actual data (full or partial block)
          size_t bytesToWrite = fileContent.gcount();
          if (bytesToWrite < BLOCK_SIZE) {
              buffer.resize(bytesToWrite);
          }
          this->disk.writeBlock(dataBlock, buffer);

          // Handle block table overflow
          if (currentEntry >= (BLOCK_SIZE / 2) - 1) {  // 127 entries per table
              // Allocate new block table
              uint16_t nextTableBlock = this->disk.allocateBlock();
              if (nextTableBlock == 0) throw std::runtime_error("No space for new block table");

              // Write next table pointer in last entry
              write2BPointer(blockTable, BLOCK_SIZE - 2, nextTableBlock);

              // Write current block table to disk
              this->disk.writeBlock(currentTableBlock, blockTable);

              // Prepare next block table
              currentTableBlock = nextTableBlock;
              blockTable.assign(BLOCK_SIZE, 0);
              currentEntry = 0;
              needsFinalWrite = false;
          } else {
              needsFinalWrite = true;
          }

          if (fileContent.eof()) break;
          buffer.resize(BLOCK_SIZE);  // Reset buffer size for next read
      }

      // Write final block table if needed
      if (needsFinalWrite) {
          write2BPointer(blockTable, BLOCK_SIZE - 2, 0);  // Terminate chain
          this->disk.writeBlock(currentTableBlock, blockTable);
      }

      std::cout << "File added successfully. Used "
                << dataBlockCount + 1  // +1 for the block table
                << " blocks.\n";
      return 0;
  } catch (const std::exception& e) {
      // Cleanup allocated blocks on failure
      this->disk.freeBlock(currentTableBlock);
      // TODO: Free any allocated data blocks
      std::cerr << "Error: " << e.what() << "\n";
      return -1;
  }

}


int FileSystem::deleteFile(const std::string fileName) {
    uint16_t blockTable = this->getFileBlockTable(fileName);
    if (blockTable == 0) {
      printf("Error: deleting a file that is not even in the directory.\n");
      return -1;
    }

    std::vector<unsigned char> buffer(256);
    std::vector<unsigned char> wipeBlock(256, 0);

    while (blockTable != 0) {
        if (this->disk.readBlock(blockTable, buffer) != 0) return -2;

        // Free data blocks
        for (uint16_t offset = 0; offset < 238; offset += 2) {
            uint16_t blockPointer = read2BPointer(buffer, offset);
            if (blockPointer == 0) break;

            this->disk.writeBlock(blockPointer, wipeBlock);
            this->disk.freeBlock(blockPointer);
        }

        // Free the block table itself
        uint16_t nextTable = read2BPointer(buffer, 238);
        this->disk.writeBlock(blockTable, wipeBlock);
        this->disk.freeBlock(blockTable);
        blockTable = nextTable;
    }

  // remove file name from the directory
  for (auto fileLog = this->directory.begin(); fileLog != this->directory.end(); ++fileLog) {
    // If they're equal, return the file content table pointer
    if (fileLog->first == fileName) {
      std::swap(*fileLog, this->directory.back());
      this->directory.pop_back();
      return this->updateDir();
    }
  }
  // Should not reach the end ...
  return -1;
}


// PRIVATE METHODS
int FileSystem::buildFileSystem() {
  this->disk.buildNewFile();
  this->cleanDir();
  return 0;
}


int FileSystem::updateDir() {
  // Obtener número de directory blocks (logs de directory / 7)
  uint16_t dirBlockCount = this->directory.size() / 7;
  ++dirBlockCount;

  // Obtener el directorio del bloque 1
  std::vector<unsigned char> oldDirBlock(256, 0);
  size_t currentDirTable = 1;
  this->disk.readBlock(currentDirTable, oldDirBlock);
  std::vector<unsigned char> newDirBlock(256, 0);
  std::vector<unsigned char> wipeBlock(256, 0);

  // para cada Directory Block
  for (uint16_t dirblock = 0; dirblock < dirBlockCount; ++dirblock) {
    // Wipe newDirBlock
    newDirBlock = wipeBlock;
    // Guardar puntero a bloque siguiente en buffer (hacerlo aunque sea 0)
    uint16_t nextDirTable = read2BPointer(oldDirBlock, 238);

    // Corner case: last dir block is free => must be freed (deletion)
    if (nextDirTable != 0 && dirBlockCount == (dirBlockCount - 1)) {
      this->disk.freeBlock(nextDirTable);
      this->disk.writeBlock(nextDirTable, wipeBlock);
      nextDirTable = 0;
    }
    // Corner case: needs new dir block => must be allocated (addition)
    if (nextDirTable == 0 && dirBlockCount == (dirBlockCount - 2)) {
      nextDirTable = this->disk.allocateBlock();
      if (nextDirTable == 0) return -1;
    }
    write2BPointer(newDirBlock, 238, nextDirTable);
    // for (uint16_t currentByte = 0; currentByte < 256; currentByte += 34) {
    size_t currentByte = 0;
    // por cada archivo (de este bloque), escribirlos en el buffer (nombre + pointer)
    for (uint16_t fileLogNum = dirblock * 7
            ; (fileLogNum < (dirblock + 1) * 7 && fileLogNum < this->directory.size())
            ; ++fileLogNum) {
      writeString32B(newDirBlock, currentByte, this->directory[fileLogNum].first);
      write2BPointer(newDirBlock, currentByte + 32, this->directory[fileLogNum].second);
      currentByte += 34;
    }
    // sobre escribir el bloque
    this->disk.writeBlock(currentDirTable, newDirBlock);
    // Cargar siguiente bloque de directory.
    currentDirTable = nextDirTable;
    this->disk.readBlock(currentDirTable, oldDirBlock);
  }
  return 0;
}


int FileSystem::loadDirectory() {
  // Read block 1 and analyze it
  // Directory is always in block 1. Start there
  uint16_t blockTable = 1;
  std::vector<unsigned char> blockBuffer(256);
  uint16_t blockTablePointer;

  // Obtain data from the directory
  while (blockTable != 0) {
    // Load directory table's content
    if (this->disk.readBlock(blockTable, blockBuffer) != 0) {
      printf("Error: failed to read block table in loadDirectory().\n");
      return -1;
    }
    for (uint16_t currentByte = 0; currentByte < 256; currentByte += 34) {
      // Get file name and pointer to block table
      std::string filename = readString32B(blockBuffer, currentByte);
      blockTablePointer = read2BPointer(blockBuffer, currentByte + 32);
      // Void pointer => no entry.
      if (blockTablePointer == 0) {
        return 0;
      // End of block directory, swith to next block directory
      } else if (currentByte == 238) {
        blockTable = read2BPointer(blockBuffer, currentByte);
        break;
      } else {
        // Store directory entry
        this->directory.emplace_back(std::move(filename), blockTablePointer);
      }
    }
  }
  if (this->directory.size() == 0) printf("Empty Directory.\n");
  return 0;
}


uint16_t FileSystem::getFileBlockTable(std::string fileName) {
  for (const std::pair<std::string, uint16_t>& fileLog : this->directory) {
    if (fileLog.first == fileName) {
      return fileLog.second;
    }
  }
  // Return invalid block number.
  return 0;
}


void FileSystem::cleanDir() {
  this->directory.clear();
  if (this->updateDir() != 0) {
    printf("Error: failed to clean the directory.");
  }
}


