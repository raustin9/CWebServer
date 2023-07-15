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
 
  // continue to send data until all data has been sent
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

// Receive a request from specified file descriptor
char*
receive_request(int fd) {
  size_t bufsize = 2048;
  char *buffer = (char*)calloc(bufsize, sizeof(char));
  if (buffer <= 0) return NULL;

  int bytes_read = 0;
  int total = 0;

  // continuously read from the file descriptor until all data has been read
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

  return buffer;
}


// Handle the logic for processing
// a connection
void
process_request(const server_t *server, int sockfd, int conn_fd, char *request) 
{
  request_t *req;
  response_t *res;
  char *response_string;
  file_t *file;
  char *file_name, *file_path, *content_type;

  // If request is empty
  if (strcmp(request, "") == 0) {
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

  // Parse the incoming request and create our response
  req = new_http_request(request);
  res = new_http_response();

  // Determine what the desired URI is
  // i.e. /app.css vs /api/...
  switch (validate_uri(req))
  {
    case 1:
      // "/"
      // Respond with index.html
      file_name = strdup("index.html");
      file_path = create_file_path(server->File_Source, file_name);
      file = read_file(file_path);
      content_type = get_content_type(file_name);
      
      set_http_response_header(res, "Status", "200 OK");
      set_http_response_header(res, "Content-Type", content_type);
      set_http_response_body(res, file->Data, file->Size);
      response_string = create_http_response_string(res);

      if (send_data(conn_fd, response_string, &res->String_Size) == -1) {
        perror("send_data");
      }

      free_file(file);
      free(file_name);
      free(file_path);
      free(content_type);
      break;

    case 2:
      // TODO: create REST API
      response_string = create_http_response_string(res);
      break;

    case 3:
      // "/requested_file"
      // Respond with the requested file
      file_name = get_file_name(req->URI);
      file_path = create_file_path(server->File_Source, file_name);
      file = read_file(file_path);
      content_type = get_content_type(file_name);
 
      set_http_response_header(res, "Status", "200 OK");
      set_http_response_header(res, "Content-Type", content_type);
      set_http_response_body(res, file->Data, file->Size);
      response_string = create_http_response_string(res);
 
      if (send_data(conn_fd, response_string, &res->String_Size) == -1) {
        perror("send_data");
      }

      free_file(file);
      free(file_name);
      free(file_path);
      free(content_type);
      break;

    default:
      // some shit fucked up
      response_string = strdup("wtf");
      break;
  }

  free(response_string);
  free_http_request(req);
  free_http_response(res);
  return;
}

// Handle the incoming requests
void
serve_on_socket(server_t *server, int sockfd) {
  int new_fd;                         // accept new connections on this fd
  char s[INET6_ADDRSTRLEN];           // buffer for IP addresses
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;

  sin_size = sizeof(their_addr);
  while (1) {
    new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    // Get the "presentation" form of the client IP
    inet_ntop(their_addr.ss_family,
      get_in_addr((struct sockaddr*)&their_addr),
      s,
      sizeof(s)
    );
    printf("server: got connection from %s\n", s);
 
    // push the file descriptor of received connection
    // onto the queue for a worker thread to grab
    pthread_mutex_lock(&mutex);
      int *n_fd = malloc(sizeof(int));
      *n_fd = new_fd;
      QueuePush(queue, (void*)n_fd);
      pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);
  }
}

// Logic for the worker threads
void*
thread_function(void* args)
{
  // pthread_t self = pthread_self();
  server_t *server = (server_t*)args;
  int conn_fd;
  qnode_t *node;
  node = NULL;
 
  while(1)
  {
    // Wait for connection to get pushed on the queue
    pthread_mutex_lock(&mutex);
    if ((node = QueuePop(queue)) == NULL)
    {
      pthread_cond_wait(&condition_var, &mutex);
      node = QueuePop(queue);
    }
    pthread_mutex_unlock(&mutex);

    // Handle connection if it grabbed one
    if (node != NULL)
    {
      // Grab the connection file descriptor
      pthread_mutex_lock(&mutex);
      conn_fd = *(int*)node->Data;
      free(node->Data);
      pthread_mutex_unlock(&mutex);
 
      // logic for request
      char *request = receive_request(conn_fd);
      if (request == NULL)
      {
        perror("webserver (receive)");
        continue;
      }

      process_request(server, 0, conn_fd, request);
      free(request);
      free(node);
      close(conn_fd);
    }
  }
  return NULL;
}

// Startup behavior for the server
void
startup()
{
  int num_threads = 15;
  queue = QueueInit();
  thread_pool = ThreadPoolInit(num_threads, thread_function);

}

int
main(int argc, char** argv) {
  int sockfd;        // file descriptor to listen on 
  int backlog = 20;  // backlog of connections to queue
  server_t *server;  // Server details
  char *port;        // port to serve on
  char *file_source; // directory source of the files to serve

  
  if (argc < 3)
  {
    fprintf(stderr, "usage: <port> <source_directory>\n");
    return 1;
  }

  port = argv[1];
  file_source = argv[2];

  (void)startup();

  server = server_create(port, backlog, file_source);
  sockfd = bind_and_listen(server);

  (void)ThreadPoolStart(thread_pool, (void*)server);

  (void)serve_on_socket(server, sockfd); // handle connections on the socket

  // lmao it should never get here
  server_free(server);
  QueueFree(queue);
  return 0;
}
