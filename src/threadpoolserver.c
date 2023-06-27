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
#include "request.h"
#include "HTTPhelpers.c"

#define PORT 8080
#define BUFFER_SIZE 1024
#define THREAD_POOL_SIZE 20

/// GLOBALS ///
tpool_t* thread_pool;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
queue_t* queue;


/// PROTOTYPES ///
int CreateSocket();
struct sockaddr_in CreateHost(int port);
char* CreateResponse(int status, char* resp_data, char* content_type);
void* HandleConnection(int newfd);
int BindAndListen(int socketfd, int port);
char* ReadFromFile(char* file_name);
void* ThreadFunction();
void ServerStartup();
// char* ParseURI(char* req_uri);
char* CreateFilePath(char* directory, char* name);
char* GetContentType(char* file_name);



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

  // Grab correct file based on request
  if (strcmp(req->URI, "/") == 0) {
    file_content = ReadFromFile("files/index.html");
    resp = CreateResponse(200, file_content, "text/html");
  } else if (req->URI[0] == '/' && strlen(req->URI) > 1) {
    char *file_path, *file_name, *content_type;
    file_name = ParseURI(req->URI);
    file_path = CreateFilePath("files/", file_name);
    content_type = GetContentType(file_name);
    
    file_content = ReadFromFile(file_path);
    resp = CreateResponse(200, file_content, content_type);
  
    free(content_type);
    free(file_name);
    free(file_path);
  } else {
    file_content = strdup("Invalid file request");
    resp = CreateResponse(404, file_content, "text/plain");
  }

  // Write to the socket
  int valwrite = write(fd, resp, strlen(resp));
  if (valwrite < 0) {
    perror("webserver (write)");
    return NULL;
  }
  close(fd);
  free(ReqInfo);

  free(req->Method);
  free(req->Version);
  free(req->URI);
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

  int* pclient = malloc(sizeof(int));

  for(;;) {
    // Accept incoming connections
    int newsocketfd = accept(socketfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
    if (newsocketfd < 0) {
      perror("webserver (accept)");
      continue;
    }
    PrintLog("connection accepted");

    // pthread_t t;

    *pclient = newsocketfd;
    pthread_mutex_lock(&mutex);
    QueuePush(queue, (void*)&newsocketfd);
    pthread_cond_signal(&condition_var);
    pthread_mutex_unlock(&mutex);
    // free(pclient);
  }

  return 1;
}

// Reads the text from a file
// This should be used to read from 
// a file asked for in an HTTP request
char*
ReadFromFile(char* file_name) {
  FILE *fp;
  long file_size;
  char* buf;

  if (access(file_name, F_OK) != 0) {
    PrintLog("Cannot find file");
    buf = (char*)malloc(strlen("FILE DOES NOT EXIST")+1);
    strcpy(buf, "FILE DOES NOT EXIST");
    return buf;
  }

  fp = fopen(file_name, "r");
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

// Get what kind of content type the HTTP response
// should be based on file extension
// ex. index.html  -> text/html
//     config.json -> application/json
char* GetContentType(char* file_name) {
  // Get file extenstion
  int i, j;
  char *file_extension, *content_type;

  file_extension = (char*)calloc(10, sizeof(char));
  for (i = 0; file_name[i] != '.' && file_name[i] != '\0'; i++);

  // No file extension -- default to plain text
  if (file_name[i] == '\0') {
    return strdup("text/plain");
  }

  for (j = 0; file_name[i] != '\0'; i++, j++) {
    file_extension[j] = file_name[i];
  }

  // Compare the file extension with 
  // different content types
  if (strcmp(file_extension, ".html") == 0 || strcmp(file_extension, ".htm") == 0) {
    content_type = strdup("text/html");
  } else if (strcmp(file_extension, ".css") == 0) {
    content_type = strdup("text/css");
  } else if (strcmp(file_extension, ".json") == 0) {
    content_type = strdup("application/json");
  } else if (strcmp(file_extension, ".aac") == 0) {
    content_type = strdup("audio/aac");
  } else if (strcmp(file_extension, ".avif") == 0) {
    content_type = strdup("image/avif");
  } else if (strcmp(file_extension, ".avi") == 0) {
    content_type = strdup("video/x-msvideo");
  } else if (strcmp(file_extension, "azw") == 0) {
    content_type = strdup("application/vnd.amazon.ebook");
  } else if (strcmp(file_extension, "bin") == 0) {
    content_type = strdup("application/octet-stream");
  } else if (strcmp(file_extension, ".bmp") == 0) {
    content_type = strdup("image/bmp");
  } else if (strcmp(file_extension, ".bz") == 0) {
    content_type = strdup("application/x-bzip");
  } else if (strcmp(file_extension, ".csv") == 0) {
    content_type = strdup("text/csv");
  } else if (strcmp(file_extension, ".doc") == 0) {
    content_type = strdup("application/msword");
  } else if (strcmp(file_extension, ".docx") == 0) {
    content_type = strdup("application/vnd.openxmlformats-officedocument.wordprocessingml.document");
  } else if (strcmp(file_extension, "gif") == 0) {
    content_type = strdup("image/gif");
  } else if (strcmp(file_extension, ".ico") == 0) {
    content_type = strdup("image/vnd.microsoft.icon");
  } else if (strcmp(file_extension, ".ics") == 0) {
    content_type = strdup("text/calendar");
  } else if (strcmp(file_extension, ".jar") == 0) {
    content_type = strdup("application/java-archive");
  } else if (strcmp(file_extension, ".jpeg") == 0 || strcmp(file_extension, ".jpg") == 0) {
    content_type = strdup("image/jpeg");
  } else if (strcmp(file_extension, ".js") == 0) {
    content_type = strdup("text/javascript");
  } else if (strcmp(file_extension, ".jsonld") == 0) {
    content_type = strdup("application/ld+json");
  } else if (strcmp(file_extension, ".mid") == 0 || strcmp(file_extension, ".midi") == 0) {
    content_type = strdup("audio/midi");
  } else if (strcmp(file_extension, ".mjs") == 0) {
    content_type = strdup("text/javascript");
  } else if (strcmp(file_extension, ".mp3") == 0) {
    content_type = strdup("audio/mpeg");
  } else if (strcmp(file_extension, ".mp4") == 0) {
    content_type = strdup("video/mp4");
  } else if (strcmp(file_extension, ".mpeg") == 0) {
    content_type = strdup("video/mpeg");
  } else if (strcmp(file_extension, ".otf") == 0) {
    content_type = strdup("font/otf");
  } else if (strcmp(file_extension, ".png") == 0) {
    content_type = strdup("image/png");
  } else if (strcmp(file_extension, ".pdf") == 0) {
    content_type = strdup("application/pdf");
  } else if (strcmp(file_extension, ".php") == 0) {
    content_type = strdup("application/x-httpd-php");
  } else if (strcmp(file_extension, ".ppt") == 0) {
    content_type = strdup("application/vnd.ms-powerpoint");
  } else if (strcmp(file_extension, ".pptx") == 0) {
    content_type = strdup("application/vnd.openxmlformats-officedocument.presentationml.presentation");
  } else if (strcmp(file_extension, ".rar") == 0) {
    content_type = strdup("application/vmd.rar");
  } else if (strcmp(file_extension, ".rtf") == 0) {
    content_type = strdup("application/rtf");
  } else if (strcmp(file_extension, ".sh") == 0) {
    content_type = strdup("application/x-sh");
  } else if (strcmp(file_extension, ".svg") == 0) {
    content_type = strdup("image/svg+xml");
  } else if (strcmp(file_extension, ".tar") == 0) {
    content_type = strdup("application/x-tar");
  } else if (strcmp(file_extension, ".ts") == 0) {
    content_type = strdup("video/mp2t");
  } else if (strcmp(file_extension, ".ttf") == 0) {
    content_type = strdup("font/ttf");
  } else if (strcmp(file_extension, ".txt") == 0) {
    content_type = strdup("text/plain");
  } else if (strcmp(file_extension, ".wav") == 0) {
    content_type = strdup("audio/wav");
  } else if (strcmp(file_extension, ".weba") == 0) {
    content_type = strdup("audio/webm");
  } else if (strcmp(file_extension, ".webm") == 0) {
    content_type = strdup("video/webm");
  } else if (strcmp(file_extension, ".webp") == 0) {
    content_type = strdup("image/webp");
  } else if (strcmp(file_extension, ".woff") == 0) {
    content_type = strdup("font/woff");
  } else if (strcmp(file_extension, ".xhtml") == 0) {
    content_type = strdup("application/xhtml+xml");
  } else if (strcmp(file_extension, ".xls") == 0) {
    content_type = strdup("application/vnd.ms-excel");
  } else if (strcmp(file_extension, ".xlsx") == 0) {
    content_type = strdup("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
  } else if (strcmp(file_extension, ".xml") == 0) {
    content_type = strdup("application/xml");
  } else if (strcmp(file_extension, ".zip") == 0) {
    content_type = strdup("application/zip");
  } else {
    PrintLog("Unsupported file extension");
    content_type = strdup("text/plain");
  }

  free(file_extension);
  return content_type;
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
