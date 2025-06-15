# Compiler
CC = gcc

# Compiler flags
CFLAGS = -std=c99 -Wall -Werror -pedantic -g

# Linker flags
LDFLAGS = -lreadline

# Executable name
TARGET = ssi

# Default target
all: $(TARGET)

# Compile 
ssi.o: ssi.c
	$(CC) $(CFLAGS) -c ssi.c -o ssi.o

# Link
$(TARGET): ssi.o
	$(CC) ssi.o -o $(TARGET) $(LDFLAGS)

# Clean
clean: 
	rm -f *.o $(TARGET)
