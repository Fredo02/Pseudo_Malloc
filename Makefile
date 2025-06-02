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

# Executable names
EXEC := $(BIN_DIR)/main
TEST_BITMAP := $(BIN_DIR)/test_bitmap
TEST_BUDDY := $(BIN_DIR)/test_buddy
TEST_ALLOCATOR := $(BIN_DIR)/test_allocator

# Default target
all: $(BIN_DIR) $(OBJ_DIR) $(EXEC)

# Link object files to create executables
$(EXEC): $(filter-out $(OBJ_DIR)/test_%, $(OBJ_FILES))
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_BITMAP): $(OBJ_DIR)/test_bitmap.o $(filter-out $(OBJ_DIR)/test_%.o, $(OBJ_FILES))
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_BUDDY): $(OBJ_DIR)/test_buddy.o $(filter-out $(OBJ_DIR)/test_%.o, $(OBJ_FILES))
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_ALLOCATOR): $(OBJ_DIR)/test_allocator.o $(filter-out $(OBJ_DIR)/test_%.o, $(OBJ_FILES))
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

# Run test_bitmap
test_bitmap: $(TEST_BITMAP)
	$(TEST_BITMAP)

# Run test_bitmap with Valgrind
valgrind_bitmap: $(TEST_BITMAP)
	valgrind $(TEST_BITMAP)

# Run test_buddy
test_buddy: $(TEST_BUDDY)
	$(TEST_BUDDY)

# Run test_buddy with Valgrind
valgrind_buddy: $(TEST_BUDDY)
	valgrind $(TEST_BUDDY)

# Run test_allocator
test_allocator: $(TEST_ALLOCATOR)
	$(TEST_ALLOCATOR)

# Run test_allocator with Valgrind
valgrind_allocator: $(TEST_ALLOCATOR)
	valgrind $(TEST_ALLOCATOR)

# Run main executable
run_main: $(EXEC)
	$(EXEC)

# Run main with Valgrind
valgrind_main: $(EXEC)
	valgrind $(EXEC)

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*

.PHONY: all clean test_bitmap test_buddy valgrind_bitmap valgrind_buddy test_allocator valgrind_allocator run_main valgrind_main