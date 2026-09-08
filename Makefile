CC = gcc
CFLAGS = -Iinclude

SRC = src/main.c src/parser.c src/builtins.c src/executor.c

all: myshell

myshell: $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o myshell

clean:
	rm -f myshell