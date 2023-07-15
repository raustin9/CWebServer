# C Multithreaded Web Server

## Purpose
This is meant to be an example of a web server that can serve a web site and application.
It is more of a learning experience than something serious, but I plan on continuing to add to it in the future.

## Description
As the title says, this is a web server built in C that uses multithreading to handle connections.
There are two versions of the server: multithreaded and threadpool.

The multithreaded version is "deprecated" as it was the first iteration. It simply spawns a new thread for each connection without limit, and joins it when complete.
This is not ideal as someone could stall my PC by spamming connections, so I built a version that uses a threadpool.

The thread-pooled version uses a thread-pool to hold a set number of threads that will handle incoming connections. 
Incoming connections are put on a queue, and when a thread is available, it will pop one off the queue and handle the connection.

## How to run
At the moment, the current build system is Make.

There are 3 different executables:
socket.c
simpleserver.c
jfslkdjjfsd.c

The default Make rule is to make all the executables, but to 
only build the server just use ```make server```

THE ONLY WAY TO KILL THE SERVER IS TO INTERRUPT IT
