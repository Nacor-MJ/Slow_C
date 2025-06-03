# CC and flags
CC = gcc
CFLAGS = -std=c11 -g3 -fno-common -Wall -Wno-switch -Werror -fms-extensions -Iinclude

# Source and object files
SRC = $(wildcard src/*.c)
LIB = $(wildcard lib/*.c)
OBJ = $(SRC:.c=.o) $(LIB:.c=.o)
TARGET = slow_c

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q src\*.o lib\*.o $(TARGET).exe 2>nul || true
	del /Q main.exe.stackdump 2>nul || true
	del /Q slow_c tmp* *.exe 2>nul || true

.PHONY: all clean
