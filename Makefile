CC=gcc
CFLAGS=-o nshell
SRC=src/main.c
BIN=nshell

all:
	@echo "Building..."
	@$(CC) $(CFLAGS) $(SRC) -o $(BIN) -lreadline

clean:
	@echo "Cleaning..."
	@rm -f $(BIN)
