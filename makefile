# Compiler
CC = gcc

# Compiler flags
CFLAGS = -std=c99 -Wall -Werror -pedantic -g

# Linker flags
LDFLAGS = -lreadline

# Executable name
TARGET = ssi

# Source and object files
SRCS = ssi.c builtins.c background.c utils.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f *.o $(TARGET)
