CC = gcc

CFLAGS = -Wall -Wextra -Iinclude

SRC = src/main.c src/parser.c src/builtins.c src/executor.c

.PHONY: all clean

all: myshell

myshell: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o myshell

clean:
	rm -f myshell