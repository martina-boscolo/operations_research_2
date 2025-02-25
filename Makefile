# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall -Wextra -O2

# Executable name
TARGET = test_gnuplot.exe

# Source files
SRCS = test_gnuplot.c
OBJS = $(SRCS:.c=.o)

# Build the executable
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Clean command
clean:
	rm -f $(TARGET) $(OBJS)

# Run target (optional)
run: $(TARGET)
	./$(TARGET)
