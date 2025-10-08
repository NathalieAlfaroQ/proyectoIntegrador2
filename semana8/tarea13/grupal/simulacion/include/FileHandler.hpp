#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <inttypes.h>
#include <utility> // Includes std::pair

class FileHandler {
 public:
  // Lifecycle
  FileHandler(const std::string& fileName);
  ~FileHandler();
  /// @brief Loads the bitmap from the current file.
  /// @return Returns 0 on success, -1 on failure.
  int loadBitMap();
  /// @brief A constructor to initialize a raw file.
  void buildNewFile();

  // Core functionality
  /// @brief Checks if there is enough storage for the requested bytes.
  /// @param bytes The number of bytes to check for.
  /// @return Returns true if there is enough storage; false otherwise.
  bool hasStorageFor(size_t bytes) const;
  /// @brief Allocates the next free block in the bit map and returns its number.
  /// @return The number of the recently allocated block.
  uint16_t allocateBlock();
  /// @brief Reads a full block from the file into the provided buffer.
  /// @param blockNumber The block number to read.
  /// @param buffer The buffer to store the read data. It will be resized to BLOCK_SIZE.
  /// @return Returns 0 on success, negative values on failure.
  int readBlock(size_t blockNumber, std::vector<unsigned char>& buffer);
  /// @brief Writes a full block to the file at a free block position.
  /// @param buffer The data to be written. If less than a full block, it will be padded with zeros.
  /// @return Returns 0 on success, -1 on failure.
  int writeBlock(size_t blockNumber, const std::vector<unsigned char>& buffer);
  /// @brief Frees in the bit map the the block with block_num.
  /// @param block_num The number of the block to be freed.
  int freeBlock(size_t blockNumber);

 private:
  // Member Variables
  /// @brief The name of the file being managed.
  std::string fileName;
  /// @brief The file stream used for file operations.
  std::fstream file;
  /// @brief A tracker for the last free block of the bit map.
  size_t last_free_block = 3;
  /// @brief A memory copy of the bitmap.
  std::vector<unsigned char> bitmap;
  /// @brief The number of free blocks available.
  size_t freeBlocks = 2048 - 3; // Total blocks - reserved blocks

  // Helper Methods
  /// @brief Searches in the bitmap for the next free blocks and returns its
  /// corresponding block number.
  /// @return The block number of the next free block.
  uint16_t getNextFreeBlock();
  /// @brief Checks in the bit map if the block is free.
  /// @param block The block to be checked.
  /// @return Returns true if the block is free; false otherwise.
  bool isBlockFree(size_t block);
  /// @brief Calculates the file offset for a given block number.
  /// @param blockNumber The block number to calculate the offset for.
  /// @return The file offset in bytes.
  size_t calculateFileOffset(size_t blockNumber) const;
  /// @brief Opens the file for reading and writing. If it does not exist, it creates it.
  /// @return Returns 0 on success, -1, -2, -3 or -4 on failure.
  int openFile();
  /// @brief Closes the currently opened file.
  /// @return Returns 0 on success, -1 on failure.
  int closeFile();
};
