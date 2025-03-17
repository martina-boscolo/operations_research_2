# Directories
INCLUDE_DIR = ./include
SRC_DIR = ./src

# Source files and object files (excluding main.c)
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(SRC_FILES:.c=.o)

# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -O3 -I$(INCLUDE_DIR)

# Executable name
TARGET = tsp.exe

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
	./$(TARGET) -n 250 -timelimit 10 -method VNS