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
#include "httputils.h"

// Return whether or not it is IPv4 or IPv6
void*
get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Ensure all data in the buffer is sent
int
send_data(int conn_fd, char *data, int *len)
{
  int total, bytes_left, n;

  total = 0;
  bytes_left = *len;
  
  while (total < *len) 
  {
    n = send(conn_fd, data+total, bytes_left, 0);
    if (n == -1) break;
    total += n;
    bytes_left -= n;
  }

  *len = total;                  // return the actual number of bytes that were sent
  return (n == -1) ? (-1) : (0); // return -1 on failure, 0 on success
}

// Clean up zombie processes
void
sigchld_handler(int s) {
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
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

// Validate the request URI
// Return 1 if they are requesting "/"
// Return 2 if they are requesting an API call
// Return 3 if they are requesting a file
int
validate_uri(request_t *req)
{
  int result;

  if (strcmp(req->URI, "/") == 0) {
    // Assume it is browser asking for index.html
    result = 1;
  } else if (strcmp(req->URI, "/api") == 0) {
    // API call rather than file request
    result = 2;
  } else {
    // Assume it is file request
    result = 3;
  }

  return result;
}

// Handle the logic for processing
// a connection
void
process_request(int sockfd, int conn_fd, char *request) 
{
  request_t *req = new_http_request(request);
  response_t *res = new_http_response();
  char *msg = strdup("Hello world");
  char *msg2 = strdup("Message 2");


  int len = strlen(msg);
  int len2 = strlen(msg2);
  char size[20];
  char *response_string;
  switch (validate_uri(req))
  {
    case 1:
      // Respond with index.html
      sprintf(size, "%d", len);
      set_http_response_header(res, "Status", "200 OK");
      set_http_response_body(res, msg, strlen(msg));
      response_string = create_http_response_string(res);
      printf("status: %s\nlength: %s\nresponse:\n%s\n", res->Headers.Status, res->Headers.Content_Length, response_string);
      if (send_data(conn_fd, res->Body.Data, &len) == -1) 
      {
        perror("send_data");
      }
      break;
    case 2:
      response_string = create_http_response_string(res);
      // API calll
      break;
    case 3:
      response_string = create_http_response_string(res);
      // Normal file request
      if (send_data(conn_fd, msg2, &len2) == -1) 
      {
        perror("send_data");
      }
      break;
    default:
      // wtf
      break;
  }

  free(msg);
  free(msg2);
  free(response_string);
  free_http_request(req);
  free_http_response(res);
  return;
}

// Handle the incoming requests
void
handle_connections(server_t *server, int sockfd) {
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
    char* request = receive(new_fd);
    if (request == NULL) {
      continue;
    }
    // printf("%s", request);

    if (fork() == 0) {
      // Child process
      close(sockfd);
      server_free(server);
      process_request(sockfd, new_fd, request);
      free(request);
      close(new_fd);
      exit(0);
    }
    close(new_fd);
    free(request);
  }
}

int
main(void) {
  int sockfd;       // file descriptor to listen on 
  server_t *server; // Server details

  server = server_create("8080", 20);
  sockfd = bind_and_listen(server);

  handle_connections(server, sockfd); // handle connections on the socket

  server_free(server);
  return 0;
}
