#pragma once
#ifndef SERVER_UTILS_
#define SERVER_UTILS_

// Structure for the server
typedef struct Server { 
  char* Port;    // port the server will listen on
  int Backlog; // the backlog of connections for the queue
  int Sock_Listen; // the socket that the server will listen on
  char* File_Source; // the directory that files will be looked up in on request
} server_t;

extern server_t* server_create(const char* port, int backlog, const char *file_source);
extern void server_free(server_t* server);
extern struct addrinfo* get_server_address(server_t* server);
extern int bind_and_listen(server_t* server);

#endif // SERVER_UTILS_
