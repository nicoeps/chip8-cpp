# Compiler
CC = g++

# Files
SRCS = $(wildcard src/*.cpp)

# Compiler flags
CFLAGS = -std=c++17

# Libraries
LIBS = -lSDL2

# Executable name
NAME = CHIP-8

# Build
all: $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LIBS) -o $(NAME) 
