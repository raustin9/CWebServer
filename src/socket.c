#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <error.h>
#include <unistd.h>

#define PORT "8080"
#define BACKLOG 40

int
main(void) {
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  struct addrinfo hints, *res;
  int sockfd, new_fd;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  (void)getaddrinfo(NULL, PORT, &hints, &res);

  // create a socket, bind it, and listen on it
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd == -1) {
    perror("webserver (create socket)");
    exit(1);
  }

  if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
    perror("webserver (bind)");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("webserver (listen)");
    exit(1);
  }

  // accept incoming connections
  addr_size = sizeof(their_addr);
  new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &addr_size);
  if (new_fd == -1) {
    perror("webserver (accept)");
    exit(1);
  }

  // Receive message
  char buffer[1024];
  int valread = recv(new_fd, buffer, 1024, 0);
  if (valread < 0) {
    perror("webserver (send)");
    exit(1);
  }
  printf("message:\n-----\n%s-----\n", buffer);

  
  char *msg = "Message received!";
  int len, bytes_sent;

  len = strlen(msg);
  bytes_sent = send(new_fd, msg, len, 0);
  if (bytes_sent < len) {
    perror("webserver (send)");
    exit(1);
  }

  close(new_fd);
  close(sockfd);

  return 0;
}
