
CC = gcc
CFLAGS = -Wall -Wextra -O2

all: console-server

console-server: console-server.o util.o stdio-handler.o log-handler.o

clean:
	rm -f console-server *.o
