# Directories
SRC_DIR := source
TEST_DIR := test
HDR_DIR := header
OBJ_DIR := object
BIN_DIR := bin

# Compiler and flags
CC := gcc
CFLAGS := -I$(HDR_DIR) -Wall -Wextra -g

# Source and object files
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
TEST_FILES := $(wildcard $(TEST_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES)) \
			 $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/%.o,$(TEST_FILES))

# Executable name
EXEC := $(BIN_DIR)/main

# Default target
all: $(BIN_DIR) $(OBJ_DIR) $(EXEC)

# Link object files to create executable
$(EXEC): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# Compile source and test files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(wildcard $(HDR_DIR)/*.h) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c $(wildcard $(HDR_DIR)/*.h) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they don't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Valgrind target
valgrind: $(EXEC)
	valgrind $(EXEC)

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean valgrind