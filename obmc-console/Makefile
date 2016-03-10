
CC = gcc
CFLAGS = -Wall -Wextra -O2

all: console-server console-client

console-server: console-server.o util.o \
		log-handler.o socket-handler.o

console-client: console-client.o util.o

clean:
	rm -f console-server console-client *.o
