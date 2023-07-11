#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "serverutils.h"

// Clean up zombie processes
void
sigchld_handler(int s) {
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
}

// Return whether or not it is IPv4 or IPv6
void*
get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Receive a message from specified file descriptor
char*
receive(int fd) {

    char *buffer = (char*)calloc(1024, sizeof(char));
    int valread = recv(fd, buffer, 1024, 0);
    if (valread == -1) {
      perror("recv");
      close(fd);
      return NULL;
    }

    return buffer;
}

// Process the incoming request
void 
process_request(int sockfd, int conn_fd, char* req) {
  char *msg = strdup("Hello, world!");
  if (send(conn_fd, msg, strlen(msg), 0) == -1) {
    perror("send");
  }

  printf("%s\n", req);
  
  free(msg);
  return;
}

// Handle the incoming requests
void
handle_connections(int sockfd) {
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int new_fd; // accept new connections on this fd
  char s[INET6_ADDRSTRLEN]; // buffer for IP addresses

  sa.sa_handler = sigchld_handler; // reap all zombie processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }


  while (1) {
    sin_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    // Get the "presentation" form of the client IP
    inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr*)&their_addr),
        s,
        sizeof(s));
    printf("server: got connection from %s\n", s);
 
    // Receive message from connection
    char* buffer = receive(new_fd);
    if (buffer == NULL) {
      continue;
    }
    printf("------------\n%s\n------------\n", buffer);

    if (fork() == 0) {
      // Child process
      close(sockfd);
      process_request(sockfd, new_fd, buffer);
      free(buffer);
      close(new_fd);
      exit(0);
    }
    close(new_fd);
    free(buffer);
  }
}

int
main(void) {
  int sockfd;       // file descriptor to listen on 
  server_t *server; // Server details

  server = server_create("8080", 20);
  sockfd = bind_and_listen(server);

  handle_connections(sockfd); // handle connections on the socket

  return 0;
}
