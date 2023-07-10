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

struct addrinfo*
get_server_address() {
  int err;
  struct addrinfo hints, *serverinfo;

  // Create server address
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;     // Hande both IPv4 and IPv6
  hints.ai_socktype = SOCK_STREAM; // Use TCP sockets rather than UDP
  hints.ai_flags = AI_PASSIVE;     

  if ((err = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
    return NULL;
  }

  return serverinfo;
}

// bind the socket
int
bind_and_listen() {
  struct addrinfo *serverinfo, *p;
  int sockfd; // file descriptor of listening socket 

  p = calloc(1, sizeof(struct addrinfo));
  serverinfo = get_server_address();

  // loop through all results and bind to the first that we can
  sockfd = bind_addr(serverinfo, p);

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }
  freeaddrinfo(serverinfo);

  if (listen(sockfd, BACKLOG) == -1) {
    perror("server (listen)");
    exit(1);
  }
  free(p);
  printf("server: listening on 8080\n");

  return sockfd;
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
void process_request(int sockfd, int conn_fd, char* req) {
  char *msg = strdup("Hello, world!");
  if (send(conn_fd, msg, strlen(msg), 0) == -1) {
    perror("send");
  }
  
  free(msg);
  return;
}

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
  int sockfd;                         // file descriptor to listen on 

  sockfd = bind_and_listen();

  handle_connections(sockfd);

  return 0;
}
