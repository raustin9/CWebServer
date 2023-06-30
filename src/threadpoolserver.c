/*
 * This is a basic multithreaded web server written in C using a Thread Pool data structure
 * This is just an example to get used to basic network proramming in C
 *
 * It can formulate a response upon request
 *
 * TODO:
 *    REFACTOR 
 *      -- ALWAYS
 *    Read html js and css files and send them as response to request 
 *      -- CAN READ FILES NOW
 *      -- ADD ABILITY TO HAVE DIFFERENT REQUESTS
 *    Send different HTML JS and CSS files upon different requests
 *      -- hostaddress/test.js -> send test.js to client
 *    Load all files to be sent to client on server startup
 *      -- HAVE FILES LOADED INTO MEMORY ALREADY WHEN THEY ARE REQUESTED
 */

#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "threadpool.h"
#include "queue.h"

#include "log.c"
#include "http.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define THREAD_POOL_SIZE 20
#define CACHE_SIZE 20

/// GLOBALS ///
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
queue_t* queue;       // queue for incoming and pending requests
char *file_cache[20]; // cache of requested file names for quick lookup
tpool_t* thread_pool; // thread pool that handles the requests on the queue
queue_t* file_name_cache;

/// PROTOTYPES ///
int CreateSocket();
struct sockaddr_in CreateHost(int port);
char* CreateResponse(int status, char* resp_data, char* content_type, uint64_t body_size);
void* HandleConnection(int newfd);
int BindAndListen(int socketfd, int port);
char* ReadFromFile(char* file_name, uint64_t* file_size);
void* ThreadFunction();
void ServerStartup();
char* CreateFilePath(char* directory, char* name);
int CheckCache(char* file_name);

// Creates a socket to listen to
int
CreateSocket() {
  int socketfd =  socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd == -1) {
    perror("webserver (socket)");
    return -1;
  }

  return socketfd;
}

// Creates the host address based on the desired port
struct sockaddr_in
CreateHost(int port) {
  struct sockaddr_in host_addr;

  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(port);
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  return host_addr;
}


// Handles the connection on the socket
void*
HandleConnection(const int fd) {
  char* buffer = calloc(BUFFER_SIZE, sizeof(char));
  char* file_content;
  char* resp;


  // Create the client address
  struct sockaddr_in client_addr;
  int client_addrlen = sizeof(client_addr);

  int sockn = getsockname(
	      fd,
	      (struct sockaddr*)&client_addr,
	      (socklen_t *)&client_addrlen
	    );

  if (sockn < 0) {
    fprintf(stderr, "%d\n", fd);
    perror("webserver (getsockname)");
    return NULL;
  }

  // Read from the socket
  int valread = read(fd, buffer, BUFFER_SIZE);
  if (valread < 0) {
    perror("webserver (read)");
    return NULL;
  }

  // Read the request
  request_t* req;
  req = ParseRequest(buffer);
  char* ReqInfo = (char*)malloc(1024);
  sprintf(
    ReqInfo,
    "[%s:%u] %s %s %s", 
    inet_ntoa(client_addr.sin_addr), 
    ntohs(client_addr.sin_port),
    req->Method,
    req->Version,
    req->URI
  );


  ReqInfo = realloc(ReqInfo, strlen(ReqInfo)+1);
  PrintLog(ReqInfo);

  uint64_t file_size = 0;
  // Grab correct file based on request
  if (strcmp(req->URI, "/") == 0) {
    file_content = ReadFromFile("files/index.html", &file_size);
    resp = CreateResponse(200, file_content, "text/html", file_size);
  } else if (req->URI[0] == '/' && strlen(req->URI) > 1) {
    char *file_path, *file_name, *content_type;
    file_name = ParseURI(req->URI);
    file_path = CreateFilePath("files/", file_name);
    content_type = GetContentType(file_name);
    
    file_content = ReadFromFile(file_path, &file_size);
    resp = CreateResponse(200, file_content, content_type, file_size);
  
    free(content_type);
    free(file_name);
    free(file_path);
  } else {
    file_content = strdup("Invalid file request");
    resp = CreateResponse(404, file_content, "text/plain", strlen(file_content)+1);
  }

  // Write to the socket
  int valwrite = write(fd, resp, strlen(resp)+file_size - strlen(file_content));
  if (valwrite < 0) {
    perror("webserver (write)");
    return NULL;
  }
  
  close(fd);
  free(ReqInfo);

  FreeRequest(req);
  free(req);
  
  free(buffer);
  free(file_content);
  free(resp);
  return NULL;
}

// Create a host with the desired port
// and bind it to the given socket
int
BindAndListen(int socketfd, int port) {
  // Create the address to bind the socket to
  struct sockaddr_in host_addr = CreateHost(port);
  int host_addrlen = sizeof(host_addr);

  // Bind the socket to the address
  if (bind(socketfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
    perror("webserver (bind)");
    return -1;
  }
  PrintLog("socket successfully bound to address");

  // Listen for incoming connections
  if (listen(socketfd, SOMAXCONN) != 0) {
    perror("webserver (listen)");
    return -1;
  }
  PrintLog("server listening for connections");

  for(;;) {
    // Accept incoming connections
    int newsocketfd = accept(socketfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
    if (newsocketfd < 0) {
      perror("webserver (accept)");
      continue;
    }
    PrintLog("connection accepted");

    pthread_mutex_lock(&mutex);
    QueuePush(queue, (void*)&newsocketfd);
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);
  }

  return 1;
}


// Reads the text from a file
// This should be used to read from 
// a file asked for in an HTTP request
// Params:
//  file_name: name of the file to read from
//  fsize: pointer to integer that we will set the size of the file to
char*
ReadFromFile(char* file_name, uint64_t* fsize) {
  FILE *fp;
  long file_size;
  char* buf;

  // Check if file exists
  if (access(file_name, F_OK) != 0) {
    PrintLog("Cannot find file");
    buf = (char*)malloc(strlen("FILE DOES NOT EXIST")+1);
    strcpy(buf, "FILE DOES NOT EXIST");
    return buf;
  }

  // Open file and check if successful
  fp = fopen(file_name, "rb");
  if (fp == NULL) {
    perror("webserver (ReadFile)");
    buf = (char*)malloc(strlen("BAD FILE READ")+1);
    strcpy(buf, "BAD FILE READ");
    return buf;
  }

  // Go to the end of file to find length of file
  // so we can create buffer of appropriate size
  fseek(fp, 0L, SEEK_END);
  file_size = ftell(fp);
  if (file_size < 0) {
    perror("webserver (file_size)");
    buf = (char*)malloc(strlen("BAD FILE READ")+1);
    strcpy(buf, "BAD FILE READ");
    return buf;
  }
  rewind(fp);
  *fsize = file_size;

  // Allocate memory for the buffer
  // with size of the file
  buf = calloc(1, file_size+1);
  if (!buf) {
    fclose(fp);
    perror("webserver (ReadFile: buffer calloc)");
    buf = (char*)malloc(strlen("BAD FILE READ")+1);
    strcpy(buf, "BAD FILE READ");
    return buf;
  }

  // Copy file contents into the buffer
  if (fread(buf, file_size, 1, fp) != 1) {
    fclose(fp);
    // free(buf);
    perror("webserver (ReadFile: fread())");
    buf = (char*)malloc(strlen("BAD FILE READ")+1);
    strcpy(buf, "BAD FILE READ");
    return buf;
  }
  fclose(fp);
  return buf;
}


// The behavior for the threads in 
// the thread pool.
// Each thread waits until they can read
// and handle a request from the queue
void*
ThreadFunction(void* args) {
  while (1) {
    qnode_t* node;
    node = NULL;
    pthread_mutex_lock(&mutex);
    if ((node = QueuePop(queue)) == NULL) {
      pthread_cond_wait(&condition_var, &mutex);
      node = QueuePop(queue);
    }
    pthread_mutex_unlock(&mutex);

    if (node != NULL) {
      HandleConnection(*(int*)node->Data);
      // free(node->Data);
      free(node);
      node = NULL;
    }
  }
  printf("Thread exiting\n");
  return NULL;
}

// Startup behavior for the server
// when it boots up
void
ServerStartup() {
  thread_pool = ThreadPoolInit(THREAD_POOL_SIZE, ThreadFunction);
  queue = QueueInit();
  file_name_cache = QueueInit();

  // Initialize thread pool
  ThreadPoolStart(thread_pool, NULL);

  // Read in the file names from the file system
  // NOT YET -- USE CACHING SYSTEM FOR FILE REQUESTS  
}


// Create the path to a file with given name
// Returns the file path which is directory/name
char* CreateFilePath(char* directory, char* name) {
    char *file_path;
    file_path = strdup("files/");
    file_path = (char*)realloc(file_path, (strlen(file_path)+1 + strlen(name)+1) * sizeof(char));
    file_path = strcat(file_path, name);

    return file_path;
}

// Check for a file name in the cache
int CheckCache(char* file_name) {
  int i;

  for (i = 0; i < CACHE_SIZE; i++) {
    if (strcmp(file_name, file_cache[i]) == 0) {
      return i;
    }
  }

  return -1;
}

/// MAIN ///
int
main() {
  ServerStartup();

  // Create a socket
  int socketfd = CreateSocket();
  if (socketfd < 0) {
    exit(1);
  } else {
    PrintLog("socket created successfully");
  }
  
  int listen = BindAndListen(socketfd, PORT);
  if (listen < 0) {
    perror("webserver (listen)");
    exit(1);
  }

  ThreadPoolJoin(thread_pool);
  free(thread_pool);

  QueueFree(queue);
  free(queue);
  return 0;
}
