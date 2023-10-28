# Compiler
CC = gcc

# Flags for the compiler: Include raylib path, enable all warnings, optimize for speed, use C99 standard
CFLAGS = -I/path/to/raylib/src -Wall -O2 -std=c99

# Linker flags: Link against raylib
LDFLAGS = -L/path/to/raylib/src -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# The final executable name
EXEC = splines

# Source files
SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJECTS)

.PHONY: all clean
