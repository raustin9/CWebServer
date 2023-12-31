CC=gcc
CFLAGS=-Wall -g -Iutils -Iinclude -pthread
BIN=bin/
SRC=src/
OBJS=obj/serverutils.o
EXECUTABLES=bin/webserver
SHOWIP=showip
SOCKET=socket
SERVER=webserver
LIB=lib/serverutils.a lib/httputils.a lib/fileutils.a

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
	$(CC) $(CFLAGS) -o $@ $< $(LIB)

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

# Library for file utilities
lib/fileutils.a: obj/fileutils.o
	ar ru $@ $<
	ranlib $@

obj/fileutils.o: utils/fileutils.c
	$(CC) $(CFLAGS) -c -o obj/fileutils.o utils/fileutils.c
