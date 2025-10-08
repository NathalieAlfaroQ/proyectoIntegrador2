#include <vector>
#include <stdlib.h>
#include <string>
#include <cinttypes>
#include <stdexcept>

#include "BlockOper.hpp"

void write2BPointer(std::vector<unsigned char>& buffer, size_t offset, uint16_t value) {
  if (offset + 2 > buffer.size()) throw std::out_of_range("Invalid offset");
  buffer[offset] = static_cast<unsigned char>(value & 0xFF);
  buffer[offset + 1] = static_cast<unsigned char>((value >> 8) & 0xFF);
}

uint16_t read2BPointer(const std::vector<unsigned char>& buffer, size_t offset) {
  if (offset + 2 > buffer.size()) throw std::out_of_range("Invalid offset");
  return static_cast<uint16_t>(buffer[offset]) |
        (static_cast<uint16_t>(buffer[offset + 1]) << 8);
}

void writeString32B(std::vector<unsigned char>& buffer, size_t pos, const std::string& str) {
  size_t index = 0;
  // Copy string characters (stop at 32 bytes)
  for (; index < 32 && index < str.size(); ++index) {
      buffer[pos + index] = static_cast<unsigned char>(str[index]);
  }
  // Pad remaining bytes with nulls
  for (; index < 32; ++index) {
      buffer[pos + index] = '\0';
  }
}

/**
 * Reads a 32-byte field as a string (stops at first null byte).
 */
std::string readString32B(const std::vector<unsigned char>& buffer, size_t pos) {
  std::string result;
  for (size_t i = 0; i < 32; ++i) {
      if (buffer[pos + i] == '\0') break;  // Stop at null terminator
      result += static_cast<char>(buffer[pos + i]);
  }
  return result;
}