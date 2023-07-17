# C Multithreaded Web Server

## Purpose
This is meant to be an example of a web server that can serve a web site and application.
It is more of a learning experience than something serious, but I plan on continuing to add to it in the future.

## Description
This is a small server that is meant to host web pages and applications as a proof of concept and exercise to learn
about network and socket programming in C. 

The design of the server is based around using a thread pool and its worker threads to handle incoming connections.
Currently, the server only handles HTTP 1.0, but I plan on implementing HTTP 1.1 soon. 

The example page that this loads by default is a basic (flawed) rendering engine made with WebGL that loads a ton of 
large image and .obj files, and this server as a decent stress test to find race conditions and other flaws in the server
while I was testing. This page can be swapped out to something else of your chosing.

## How to run
At the moment, the current build system is Make.
Run the command
```make``` or ```make server```
to build the server. This will create the executable bin/webserver.

There is a script called ```scripts/run.sh``` that you can run that will use valgrind and some of its options to test 
the server for leaks, but you can also just run the executable on its own. The ```run.sh``` will default to listening on 
port ```8088``` and source the files to host from ```files```. To change this, you can just edit the script by editing the
arguments to bin/webserver.

THE ONLY WAY TO KILL THE SERVER IS TO INTERRUPT IT
