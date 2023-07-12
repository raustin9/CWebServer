#include "serverutils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Generate a new server with desired parameters
server_t*
server_create(const char* port, int backlog, const char *file_source) {
  server_t* s = (server_t*)calloc(1, sizeof(server_t));

  s->Port = strdup(port);
  s->Backlog = backlog;
  s->File_Source = strdup(file_source);

  return s;
}

// Free the memembers of the server
// along with the server itself
void 
server_free(server_t* server) 
{
  free(server->Port);
  free(server);
}


// Get the information for the server
struct addrinfo*
get_server_address(server_t* server) {
  int err;
  struct addrinfo hints, *serverinfo;

  // Create server address
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;     // Hande both IPv4 and IPv6
  hints.ai_socktype = SOCK_STREAM; // Use TCP sockets rather than UDP
  hints.ai_flags = AI_PASSIVE;     

  if ((err = getaddrinfo(NULL, server->Port, &hints, &serverinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return NULL;
  }

  return serverinfo;
}

// Bind socket to first address available 
// to the desired socket
// returns the file descriptor of the socket that was binded
int
bind_addr(struct addrinfo *serverinfo, struct addrinfo *p) {
  int yes = 1, sockfd;

  // loop through all results and bind to the first that we can
  for (p = serverinfo; p != NULL; p = p->ai_next) {
    // Create socket from the address
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: (socket)");
      continue;
    }

    // Allow reuse of the socket
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("server (setsockopt)");
      exit(1);
    }

    // Attemp to bind, and if successful break the loop
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server (bind)");
      continue;
    }

    break;
  }

  return sockfd;
}


// bind the socket
int
bind_and_listen(server_t* server) {
  struct addrinfo *serverinfo, *p;
  int sockfd; // file descriptor of listening socket 

  p = calloc(1, sizeof(struct addrinfo));
  serverinfo = get_server_address(server);

  // loop through all results and bind to the first that we can
  sockfd = bind_addr(serverinfo, p);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }
  freeaddrinfo(serverinfo);

  if (listen(sockfd, server->Backlog) == -1) {
    perror("server (listen)");
    exit(1);
  }
  free(p);
  printf("server: listening on 8080\n");

  return sockfd;
}


