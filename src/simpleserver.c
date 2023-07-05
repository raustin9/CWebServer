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

#define PORT "8080"
#define BACKLOG 10

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

// Bind socket to first address available 
// to the desired socket
int
bind_addr(struct addrinfo *serverinfo, struct addrinfo *p) {
  int yes = 1, sockfd;

  // loop through all results and bind to the first that we can
  for (p = serverinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: (socket)");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("server (setsockopt)");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server (bind)");
      continue;
    }

    break;
  }

  return sockfd;
}

int
main(void) {
  int sockfd, new_fd; // listen on sockfd, accept on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all results and bind to the first that we can
  sockfd = bind_addr(servinfo, p);
  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1) {
    perror("server (listen)");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // reap all zombie processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: listening on 8080\n");

  while (1) {
    sin_size = sizeof(their_addr);
    new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }


    inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr*)&their_addr),
        s,
        sizeof(s));
    printf("server: got connection from %s\n", s);
    char buffer[1024];
    int valread = recv(new_fd, buffer, 1024, 0);
    if (valread == -1) {
      perror("recv");
      close(new_fd);
      continue;
    }
    printf("------------\n%s\n------------\n", buffer);

    if (fork() == 0) {
      // Child process
      close(sockfd);
      if (send(new_fd, "Hello, world!", 13, 0) == -1) {
        perror("send");
      }
      close(new_fd);
      exit(0);
    }
    close(new_fd);
  }

  return 0;
}
