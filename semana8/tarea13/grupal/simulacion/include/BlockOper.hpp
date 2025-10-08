#pragma once

/// @brief Writes a 2 byte pointer to the buffer in the position pos.
/// @param buffer The buffer to be modified.
/// @param pos The position where to write.
/// @param value The value to be written.
void write2BPointer(std::vector<unsigned char>& buffer, size_t pos, uint16_t value);
/// @brief Reads a 2 byte pointer from the buffer in the position pos.
/// @param buffer The buffer to be accessed.
/// @param pos The position where to read.
/// @return Returns the value of the pointer read.
uint16_t read2BPointer(const std::vector<unsigned char>& buffer, size_t pos);
/// @brief Writes a string of max 32 bytes into the buffer in the position pos.
/// @param buffer The buffer to be modified.
/// @param pos The position where to write.
/// @param value The value to be written.
void writeString32B(std::vector<unsigned char>& buffer, size_t pos, const std::string& str);
/// @brief Reads a string of 32 bytes into the buffer in the position pos.
/// @param buffer The buffer to be accessed.
/// @param pos The position where to read.
/// @return The string read.
std::string readString32B(const std::vector<unsigned char>& buffer, size_t pos);