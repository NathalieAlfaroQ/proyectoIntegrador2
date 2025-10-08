#pragma once

#include <string>
#include <vector>

#include "FileHandler.hpp"

constexpr const char* FILE_NAME = "file_sys_1.txt";

/// @brief Manages the overall logic of a file system.
class FileSystem {
 public:
  // LIFECYCLE
  FileSystem(const std::string& fileName = FILE_NAME);
  ~FileSystem();
  /// @brief builds an empty file system with the basic logic information.
  /// @return returns 0 on success.
  int buildFileSystem();

  // CORE FUNCTIONALITY
  /// @brief Gets the directory in a string.
  /// @return returns the information stored in the directory.
  std::string getDirectory();
  /// @brief Returns the directory as an unordered list in HTML.
  /// @return The string with the formatted HTML.
  std::string getDirectoryHTML();
  /// @brief Gets the file with the name fileName. If not found, returns "File
  /// Not Found"
  /// @param fileName The name of the file.
  /// @return returns the contents of the file in a string.
  std::string getFile(std::string fileName);
  /// @brief Adds a file to the directory
  /// @param name The name of the file to be added
  /// @param fileContent The contents of the file
  /// @return retuns 0 on success.
  int addFile(std::string name, std::istream& fileContent);
  /// @brief Deletes a file from the directory
  /// @param name The name of the file to be deleted
  /// @return retuns 0 on success.
  int deleteFile(std::string name);


 private:
  // MEMBER VARIABLES
  /// @brief Used to write and read blocks from memory.
  FileHandler disk;
  /// @brief A vector with the pairs of file names and BlockTable block numbers.
  std::vector<std::pair<std::string, uint16_t>> directory;

  // UTILITY METHODS
  /// @brief Persist the directory after a modification.
  /// @return returns 0 on success.
  int updateDir();
  /// @brief Loads the directory from the file
  /// @return returns 0 on success.
  int loadDirectory();
  /// @brief Gets the number of the block with the blockTable of the file.
  /// @param fileName The name of the file to be searched
  /// @return returns the number of the block with the blockTable of the file.
  uint16_t getFileBlockTable(std::string fileName);
  /// @brief removes all the data in the directory.
  void cleanDir();
};
