C=gcc
CFLAGS=-o nshell

all:
	@echo "Build..."
	@$(C) $(CFLAGS) src/main.c

clean:
	@echo "Clean..."
	@rm main.out

