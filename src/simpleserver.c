#include <pthread.h>
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
#include "fileutils.h"
#include "queue.h"
#include "threadpool.h"

#define THREAD_POOL_SIZE 20

// Globals
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
queue_t *queue;
tpool_t *thread_pool;


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
send_data(int conn_fd, char *data, size_t *len)
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
  size_t bufsize = 2048;
  char *buffer = (char*)calloc(bufsize, sizeof(char));
  if (buffer <= 0) return NULL;

  int bytes_read = 0;
  int total = 0;

  while (bytes_read = recv(fd, buffer+total, bufsize, 0), bytes_read == bufsize)
  {
    total += bytes_read;
    char* reallocated = (char*)realloc(buffer, bufsize + total);

    if (reallocated <= 0)
    {
      free(buffer);
      return NULL;
    }

    buffer = reallocated;
    memset(buffer + total, 0, bufsize);
  }

//    int valread = recv(fd, buffer, 1024, 0);
//    if (valread == -1) {
//      free(buffer);
//      perror("recv");
//      // close(fd);
//      return NULL;
//    }

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
process_request(const server_t *server, int sockfd, int conn_fd, char *request) 
{
  if (strcmp(request, "") == 0) 
  {
    response_t *res = new_http_response();
    unsigned char *resp_str = (unsigned char*)strdup("ping");
    set_http_response_header(res, "Status", "200 OK");
    set_http_response_header(res, "Content-Type", "text/plain");
    set_http_response_body(res, resp_str, 4);
    char *response_string = create_http_response_string(res);

    if (send_data(conn_fd, response_string, &res->String_Size) == -1) 
    {
      perror("send_data");
    }
    free(resp_str);
    free(response_string);
    free_http_response(res);
    return;
  }

  request_t *req = new_http_request(request);
  response_t *res = new_http_response();

  char *response_string;
  file_t *file;
  char *file_name, *file_path, *content_type;
  // char *file_path = strdup("files/");

  switch (validate_uri(req))
  {
    case 1:
      // Respond with index.html
      file_name = strdup("index.html");
      file_path = create_file_path(server->File_Source, file_name);
      file = read_file(file_path);
      content_type = get_content_type(file_name);
      
      set_http_response_header(res, "Status", "200 OK");
      set_http_response_header(res, "Content-Type", content_type);
      set_http_response_body(res, file->Data, file->Size);
      response_string = create_http_response_string(res);

      // printf("sending on %d\n", conn_fd);
      if (send_data(conn_fd, response_string, &res->String_Size) == -1) 
      {
        perror("send_data");
      }

      free_file(file);
      free(file_name);
      free(file_path);
      free(content_type);
      break;
    case 2:
      response_string = create_http_response_string(res);
      // API calll
      break;
    case 3:
      // Respond with requested file
      file_name = get_file_name(req->URI);
      file_path = create_file_path(server->File_Source, file_name);
      file = read_file(file_path);
      content_type = get_content_type(file_name);
 
      set_http_response_header(res, "Status", "200 OK");
      set_http_response_header(res, "Content-Type", content_type);
      set_http_response_body(res, file->Data, file->Size);
      response_string = create_http_response_string(res);
 
      // printf("sending on %d\n", conn_fd);
      if (send_data(conn_fd, response_string, &res->String_Size) == -1) 
      {
        perror("send_data");
      }

      free_file(file);
      free(file_name);
      free(file_path);
      free(content_type);
      break;
    default:
      // wtf
      break;
  }

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
 
    pthread_mutex_lock(&mutex);
    int *n_fd = malloc(sizeof(int));
    *n_fd = new_fd;
    QueuePush(queue, (void*)n_fd);
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);
    
   /*
    // FOR FORKING PROCESSES
    // Receive message from connection
    char* request = receive(new_fd);
    if (request == NULL) {
      continue;
    }
    // printf("%s", request);

    if (fork() == 0) {
      // Child process
      close(sockfd);
      process_request(server, sockfd, new_fd, request);
      server_free(server);
      free(request);
      close(new_fd);
      exit(0);
    }
    close(new_fd);
    free(request); 
     */
  }
}

void*
thread_function(void* args)
{
  // pthread_t self = pthread_self();
  server_t *server = (server_t*)args;
  int fd;
  while(1)
  {
    qnode_t *node;
    node = NULL;

    // Wait for connection to get pushed on the queue
    pthread_mutex_lock(&mutex);
    if ((node = QueuePop(queue)) == NULL)
    {
      pthread_cond_wait(&condition_var, &mutex);
      node = QueuePop(queue);
    }
    pthread_mutex_unlock(&mutex);

    // If it got a connection
    // handle it
    if (node != NULL)
    {
      pthread_mutex_lock(&mutex);
      fd = *(int*)node->Data;
      free(node->Data);
      pthread_mutex_unlock(&mutex);
      // logic for request
      // int fd = *(int*)node->Data;
      // printf("%lu -- fd: %d\n", self,  *fd);
      char *request = receive(fd);
      if (request == NULL)
      {
        perror("webserver (receive)");
        continue;
      }
      process_request(server, 0, fd, request);
      free(request);
      free(node);
      // printf("closing %d\n", fd);
      close(fd);
    }
  }
  return NULL;
}

void
startup()
{
  queue = QueueInit();
  thread_pool = ThreadPoolInit(THREAD_POOL_SIZE, thread_function);

}

int
main(int argc, char** argv) {
  int sockfd;       // file descriptor to listen on 
  server_t *server; // Server details
  char *port, *file_source;
  
  if (argc < 3)
  {
    fprintf(stderr, "usage: <port> <dir_source>\n");
    return 1;
  }

  port = strdup(argv[1]);
  file_source = strdup(argv[2]);
 
  (void)startup();

  server = server_create(port, 20, file_source);
  sockfd = bind_and_listen(server);

  (void)ThreadPoolStart(thread_pool, (void*)server);

  (void)handle_connections(server, sockfd); // handle connections on the socket

  server_free(server);
  QueueFree(queue);
  return 0;
}
