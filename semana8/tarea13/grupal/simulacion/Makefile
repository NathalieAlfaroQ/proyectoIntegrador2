# Compiler settings
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS := 
DEBUG_FLAGS := -g -O0 -DDEBUG
RELEASE_FLAGS := -O3 -DNDEBUG

# Directory structure
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin
FILES_DIR := files

# Source files - busca tanto .cc como .cpp
CC_SRCS := $(wildcard $(SRC_DIR)/*.cc)
CPP_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
SRCS := $(CC_SRCS) $(CPP_SRCS)

# Object files
CC_OBJS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(CC_SRCS))
CPP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SRCS))
OBJS := $(CC_OBJS) $(CPP_OBJS)

# Target name
TARGET := $(BIN_DIR)/simulacion

# Ensure output directories exist
$(shell mkdir -p $(BUILD_DIR) $(BIN_DIR) $(FILES_DIR))

# Default target (debug build)
all: debug

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Release build
release: CXXFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Build main executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Build completado: $(TARGET)"

# Pattern rule for .cc files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Pattern rule for .cpp files  
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Run the simulation
run: debug
	./$(TARGET)

# Debug run
debug-run: debug
	gdb --args $(TARGET)

# Show source files found
info:
	@echo "CC sources: $(CC_SRCS)"
	@echo "CPP sources: $(CPP_SRCS)"
	@echo "All sources: $(SRCS)"
	@echo "Objects: $(OBJS)"

.PHONY: all debug release clean run debug-run info	