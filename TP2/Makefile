SRC = $(wildcard *.c)
BIN = main

CC = gcc
CFLAGS += -Wall -Wextra -g -fdiagnostics-color=always
#CFLAGS += --std=c99

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@
	
clean:
	rm -f $(BIN)
	rm -f *.o
	rm -f *~

indent:
	indent -linux -i4 -nut -ts2 *.c

.PHONY: all clean indent
