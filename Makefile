CC=gcc
CFLAGS=-Wall -pthread -g
EXECUTABLES= bin/poolserver bin/multiserver
OBJS=obj/pool.o
INCLUDE=-I include/ -I lib/
BIN=bin/

all:$(EXECUTABLES)

clean:
	rm -f obj/* bin/*

pool: bin/poolserver

bin/poolserver: obj/pool.o
	$(CC) $(CFLAGS) -o bin/poolserver obj/pool.o 

obj/pool.o: src/threadpoolserver.c
	$(CC) $(CFLAGS) $(INCLUDE) -o obj/pool.o -c src/threadpoolserver.c


multi: bin/multiserver

bin/multiserver: obj/multi.o
	$(CC) $(CFLAGS) -o bin/multiserver obj/multi.o

obj/multi.o:
	$(CC) $(CFLAGS) $(INCLUDE) -o obj/multi.o -c src/multithreadserver.c
