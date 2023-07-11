CC=gcc
CFLAGS=-Wall -g -Iutils
BIN=bin/
SRC=src/
OBJS=obj/serverutils.o
EXECUTABLES=bin/showip bin/socket bin/simpleserver
SHOWIP=showip
SOCKET=socket
SERVER=simpleserver
LIB=lib/serverutils.a lib/httputils.a

all: $(EXECUTABLES)

clean:
	rm -f obj/* bin/* lib/*

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


# Compile the server
server: bin/$(SERVER)

bin/$(SERVER): src/$(SERVER).c $(LIB)
	$(CC) $(CFLAGS)  -o $@ $< $(LIB)

# Library for server utilities
lib/serverutils.a: obj/serverutils.o
	ar ru $@ $<
	ranlib $@

obj/serverutils.o: utils/serverutils.c
	$(CC) $(CFLAGS) -c -o obj/serverutils.o utils/serverutils.c

# Library for http utilities
lib/httputils.a: obj/httputils.o
	ar ru $@ $<
	ranlib $@

obj/httputils.o: utils/httputils.c
	$(CC) $(CFLAGS) -c -o obj/httputils.o utils/httputils.c
