#pragma once
#ifndef SERVER_UTILS_
#define SERVER_UTILS_

// Structure for the server
typedef struct Server { 
  char* Port;    // port the server will listen on
  int Backlog; // the backlog of connections for the queue
} server_t;

extern server_t* server_create(char* port, int backlog);
extern struct addrinfo* get_server_address(server_t* server);
extern int bind_and_listen(server_t* server);

#endif // SERVER_UTILS_
