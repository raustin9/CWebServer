CC=gcc
CFLAGS=-Wall -g
BIN=bin/
SRC=src/
EXECUTABLES=bin/showip bin/socket bin/simpleserver
SHOWIP=showip
SOCKET=socket
SERVER=simpleserver

all: $(EXECUTABLES)

clean:
	rm -f obj/* bin/*

showip: bin/$(SHOWIP)

bin/showip: obj/showip.o
	$(CC) $(CFLAGS) -o bin/$(SHOWIP) obj/$(SHOWIP).o

obj/showip.o: src/$(SHOWIP).c
	$(CC) $(CFLAGS) -o obj/showip.o -c src/$(SHOWIP).c


socket: bin/$(SOCKET)

bin/$(SOCKET): obj/$(SOCKET).o
	$(CC) $(CFLAGS) -o bin/$(SOCKET) obj/$(SOCKET).o

obj/$(SOCKET).o: src/$(SOCKET).c
	$(CC) $(CFLAGS) -o obj/$(SOCKET).o -c src/$(SOCKET).c


server: bin/$(SERVER)

bin/$(SERVER): obj/$(SERVER).o
	$(CC) $(CFLAGS) -o bin/$(SERVER) obj/$(SERVER).o

obj/$(SERVER).o: src/$(SERVER).c
	$(CC) $(CFLAGS) -o obj/$(SERVER).o -c src/$(SERVER).c
