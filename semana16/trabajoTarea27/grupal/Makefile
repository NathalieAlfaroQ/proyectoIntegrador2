CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iincludes
LDFLAGS := -lssl -lcrypto

SRC_DIR = src
INC_DIR = includes
OBJ_DIR = obj
BIN_DIR = bin

INCLUDE_SOURCES = $(wildcard $(INC_DIR)/*.cc)
INCLUDE_OBJECTS = $(patsubst $(INC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(INCLUDE_SOURCES))

PROGRAMS = cache.out client.out figureServer.out forkServer.out
TARGETS = $(addprefix $(BIN_DIR)/, $(PROGRAMS))

all: $(TARGETS)

$(BIN_DIR)/%: $(SRC_DIR)/%.cc $(INCLUDE_OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(INC_DIR)/%.cc $(INC_DIR)/%.h
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean