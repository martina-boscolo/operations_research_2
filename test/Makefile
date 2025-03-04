# Directories
INCLUDE_DIR = ../include
SRC_DIR = ../src
TEST_DIR = ./src

# Source files and object files (excluding main.c)
SRC_FILES = $(filter-out $(SRC_DIR)/main.c, $(wildcard $(TEST_DIR)/*.c) $(wildcard $(SRC_DIR)/*.c))
OBJ_FILES = $(SRC_FILES:.c=.o)

# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -O2 -I$(INCLUDE_DIR)

# Executable name
TARGET = test.exe

# Rule to build the final executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(TARGET)

# Rule to build .o files from .c files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# Clean up generated files
clean:
	rm -f $(TARGET) $(OBJ_FILES) 

# Rule to clean and rebuild
rebuild: clean $(TARGET)

# Run target
run: $(TARGET)
	./$(TARGET) -n 10