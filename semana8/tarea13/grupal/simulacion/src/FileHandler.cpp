#include <cstring>
#include <stdexcept>
#include <cassert>
#include <inttypes.h>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include "FileHandler.hpp"

constexpr const size_t BLOCK_COUNT = 2048; // Cantidad de bloques
constexpr const size_t BLOCK_SIZE = 256; // Cantidad de bloques
constexpr size_t TOTAL_FILE_SIZE = BLOCK_SIZE * BLOCK_COUNT; // 256 * 2048

// LIFECYCLE
FileHandler::FileHandler(const std::string& fileName)
        : fileName(fileName) {
  if (openFile() != 0) {
    throw std::runtime_error("FileHandler::Failed to open file");
  }
  this->bitmap.resize(256);
  if (this->loadBitMap() != 0) {
    printf("File is not a File System. Proceeding to make it one...\n");
    this->buildNewFile();
  }
  printf("The file seems to be valid.\n");
}


FileHandler::~FileHandler() {
  if (closeFile() != 0) {
    throw std::runtime_error("Failed to close file");
  }
}

int FileHandler::loadBitMap() {
  // Checks for the first block to be empty (all zeros)
  std::vector<unsigned char> block0(256);
  if (this->readBlock(0, block0) != 0) return -1;

  // Checks for a well made bitmap
  if (this->readBlock(2, this->bitmap) != 0) return -1;
  if (this->isBlockFree(2)) return -1;
  if (this->isBlockFree(1)) return -1;
  if (this->isBlockFree(0)) return -1;

  return 0;
}

void FileHandler::buildNewFile() {
  // Initialize bitmap
  this->bitmap = std::vector<unsigned char>(256, 0xFF); // Start with all blocks "free" (1=free)

  // Mark blocks 0-2 as used (bit=0)
  for (int index = 0; index <= 2; ++index) {
      int byte_pos = index / 8;
      int bit_pos = index % 8;
      this->bitmap[byte_pos] &= ~(1 << bit_pos);
  }
  // Write to Block 2
  this->writeBlock(2, this->bitmap);

  // wipe the first and second block
  std::vector<unsigned char> zeroBlock(256, 0x00);
  this->writeBlock(0, zeroBlock);
  this->writeBlock(1, zeroBlock);
  printf("New file system created successfully.\n");
}

// CORE FUNCTIONALITY
bool FileHandler::hasStorageFor(size_t bytes) const {
  size_t requiredBlocks = (bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
  return requiredBlocks <= freeBlocks;
}

int FileHandler::readBlock(size_t blockNumber, std::vector<unsigned char>& buffer) {
  // Validate block number first
  if (blockNumber >= BLOCK_COUNT) {
      std::cerr << "Error: Invalid block number " << blockNumber
                << " (max " << BLOCK_COUNT << ")\n";
      return -1;
  }

  // Check file state thoroughly
  if (!file.is_open()) {
      std::cerr << "Error: File not open\n";
      return -2;
  }
  if (!file.good()) {
      std::cerr << "Error: File in bad state (failbit: " << file.fail()
                << ", badbit: " << file.bad() << ", eofbit: " << file.eof() << ")\n";
      return -3;
  }

  // Calculate and validate offset
  size_t offset = calculateFileOffset(blockNumber);
  if (offset % BLOCK_SIZE != 0) {
      std::cerr << "Error: Invalid block alignment\n";
      return -4;
  }

  // Prepare buffer
  buffer.resize(BLOCK_SIZE);

  // Perform seek and verify
  file.seekg(offset);
  if (file.fail()) {
      std::cerr << "Error: Seek failed to position " << offset << "\n";
      return -5;
  }

  // Perform read
  file.read(reinterpret_cast<char*>(buffer.data()), BLOCK_SIZE);
  const size_t bytesRead = file.gcount();

  // Handle read results
  if (file.eof()) {
      std::cerr << "Warning: Reached EOF after reading " << bytesRead << " bytes\n";
  }
  if (file.fail() && !file.eof()) {
      std::cerr << "Error: Read operation failed\n";
      return -6;
  }

  return (bytesRead == BLOCK_SIZE) ? 0 : -7;
}

int FileHandler::freeBlock(size_t block_num) {
  // Rango válido
  if (block_num <= 2 || block_num >= 2048) return -1;

  // Actualizar el bitmap
  size_t byte_pos = block_num / 8;
  size_t bit_pos = block_num % 8;
  bitmap[byte_pos] |= (1 << bit_pos);
  this->writeBlock(2, this->bitmap);

  // Llevar registro del último bloque libre
  if (block_num < last_free_block) {
      last_free_block = block_num; // Now the earliest available block
  }
  return 0;
}


// UTILITY METHODS
uint16_t FileHandler::allocateBlock() {
  size_t block = getNextFreeBlock();
  if (block != 0) {
      // Update bitmap
      size_t byte = block / 8;
      size_t bit = block % 8;
      bitmap[byte] &= ~(1 << bit);  // Clear bit
      this->writeBlock(2, bitmap);    // Persist

      //Todo(): check for correctness
      // wipe the block
      this->writeBlock(block, std::vector<unsigned char>(256, 0x00));
  }
  return block;
}

int FileHandler::writeBlock(size_t blockNumber, const std::vector<unsigned char>& buffer) {
  if (blockNumber >= BLOCK_COUNT) return -1;
  if (!file.is_open()) return -2;

  // Seek to EXACT block position
  const size_t offset = blockNumber * BLOCK_SIZE;
  file.seekp(offset);
  if (file.fail()) return -3;

  // Write ENTIRE block (pad with zeros if needed)
  std::vector<unsigned char> blockData(BLOCK_SIZE, 0);
  const size_t bytesToWrite = std::min(buffer.size(), BLOCK_SIZE);
  std::copy_n(buffer.begin(), bytesToWrite, blockData.begin());

  file.write(reinterpret_cast<const char*>(blockData.data()), BLOCK_SIZE);
  file.flush();

  return file.good() ? 0 : -4;
}

uint16_t FileHandler::getNextFreeBlock() {
  // Quick check near last allocation (spatial locality)
  for (uint16_t i = last_free_block; i < 2048; ++i) {
      if (isBlockFree(i)) {
          last_free_block = i + 1;
          return i;
      }
  }

  // Full scan if no blocks found after last_free_block
  for (uint16_t i = 3; i < last_free_block; ++i) {
      if (isBlockFree(i)) {
          last_free_block = i + 1;
          return i;
      }
  }
  return 0;
}

bool FileHandler::isBlockFree(size_t block) {
  size_t byte = block / 8;
  size_t bit = block % 8;
  return (bitmap[byte] >> bit) & 1;
}


size_t FileHandler::calculateFileOffset(size_t blockNumber) const {
  return blockNumber * BLOCK_SIZE;
}

int FileHandler::openFile() {
  // Try opening existing file first
  file.open(this->fileName, std::ios::binary | std::ios::in | std::ios::out);

  if (file.is_open()) {
      // Verify existing file size
      file.seekg(0, std::ios::end);
      size_t actualSize = file.tellg();
      file.seekg(0); // Rewind

      if (actualSize != TOTAL_FILE_SIZE) {
          std::cerr << "Error: Invalid file size (" << actualSize
                    << " bytes, expected " << TOTAL_FILE_SIZE << ")\n";
          file.close();
          return -2; // Special error code for size mismatch
      }
      return 0;
  }

  // Create new file with correct size
  file.open(fileName, std::ios::binary | std::ios::out);
  if (!file.is_open()) {
      return -1; // Failed to create
  }

  // Pre-size the file
  file.seekp(TOTAL_FILE_SIZE - 1);
  file.write("\0", 1);
  file.flush();

  // Verify the size was set
  file.seekp(0, std::ios::end);
  if (file.tellp() != TOTAL_FILE_SIZE) {
      file.close();
      std::filesystem::remove(fileName); // Clean up
      return -3; // Failed to resize
  }

  file.close();

  // Reopen with proper flags
  file.open(fileName, std::ios::binary | std::ios::in | std::ios::out);
  return file.is_open() ? 0 : -4;
}

int FileHandler::closeFile() {
  if(this->file.is_open()) {
    this->file.close();
    return 0;
  } else {
    return -1;
  }
}