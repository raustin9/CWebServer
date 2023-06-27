/*
 * This is a basic multithreaded web server written in C
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
 *    Impliment a thread pool
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

#define PORT 8080
#define BUFFER_SIZE 1024
#define THREAD_POOL_SIZE 20

// Information from HTTP Request
typedef struct HTTPRequestData {
  char* Method;
  char* URI;
  char* Version;
} request_t;

/// PROTOTYPES ///
int CreateSocket();
struct sockaddr_in CreateHost(int port);
char* GetStatusMsg(int status);
char* CreateResponse(int status, char* resp_data);
void* HandleConnection(void* newfd);
int BindAndListen(int socketfd, int port);
char* ReadFromFile(char* file_name);
void PrintLog(char* text);
request_t* ParseRequest(char* request);
char* FormatTime();

// Gets the time and formats it properly to use
// with the log
char*
FormatTime() {
  char* output;
  time_t rawtime;
  struct tm* timeinfo;
  
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  output = malloc(20); // 20 bytes for now
  sprintf(
    output, 
    "[%d-%d-%d | %d:%d:%d]", 
    timeinfo->tm_mday, 
    timeinfo->tm_mon+1, 
    timeinfo->tm_year+1900, 
    timeinfo->tm_hour, 
    timeinfo->tm_min, 
    timeinfo->tm_sec
  );

  return output;
}

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

// Gets the status message based on the status code
// ex: 200 => "200 OK"
char*
GetStatusMsg(int status) {
  char* rv;
  rv = malloc(100);

  switch (status) {
    case 200:
      strcpy(rv, "200 OK");
      break;

    case 201:
      strcpy(rv, "201 Created");
      break;

    case 202:
      strcpy(rv, "202 Accepted");
      break;

    case 203:
      strcpy(rv, "203 Non-Authoritative Information");
      break;

    case 204:
      strcpy(rv, "204 No Content");
      break;

    default:
      strcpy(rv, "404 Not Found");
      break;
  }

  return rv;
}

// Generetes and HTTP response based on the status code and desired response body
char*
CreateResponse(int status, char* resp_data) {
  char* rv;
  char* statusmsg;

  statusmsg = GetStatusMsg(200);

  rv = malloc(500 + strlen(resp_data)+1 + strlen(statusmsg)+1);

  sprintf(
    rv, 
    "HTTP/1.1 %s\r\n"
    "Server: webserver-c\r\n"
    "Content-Type: text/html\r\n\r\n"
    "%s\r\n",
    statusmsg,
    resp_data
  );

  free(statusmsg); 
  return rv;
}

// Handles the connection on the socket
void*
HandleConnection(void* newfd) {
  int fd = * (int*)newfd;
  char buffer[BUFFER_SIZE];
  char* file_content = ReadFromFile("files/index.html");
  char* resp = CreateResponse(200, file_content);

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
  request_t* req = ParseRequest(buffer);
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
  PrintLog(ReqInfo);
  fflush(stdout);

  // Write to the socket
  int valwrite = write(fd, resp, strlen(resp));
  if (valwrite < 0) {
    perror("webserver (write)");
    return NULL;
  }

  close(fd);
  free(req);
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
  fflush(stdout);

  // Listen for incoming connections
  if (listen(socketfd, SOMAXCONN) != 0) {
    perror("webserver (listen)");
    return -1;
  }
  PrintLog("server listening for connections");
  fflush(stdout);

  for (;;) {
    // Accept incoming connections
    int newsocketfd = accept(socketfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
    if (newsocketfd < 0) {
      perror("webserver (accept)");
      continue;
    }
    PrintLog("connection accepted");
    fflush(stdout);

    pthread_t t;
    int* pclient = malloc(sizeof(int));
    *pclient = newsocketfd;
    // QueuePush(queue, (void*)pclient);
    pthread_create(&t, NULL, HandleConnection, (void*)pclient);
  }

  return 1;
}

char*
ReadFromFile(char* file_name) {
  FILE *fp;
  long file_size;
  char* buf;

  fp = fopen(file_name, "r");
  if (fp == NULL) {
    perror("webserver (ReadFile)");
    return "BAD FILE READ";
  }

  // Go to the end of file to find length of file
  // so we can create buffer of appropriate size
  fseek(fp, 0L, SEEK_END);
  file_size = ftell(fp);
  if (file_size < 0) {
    perror("webserver (file_size)");
    return "BAD FILE READ";
  }
  rewind(fp);

  // Allocate memory for the buffer
  // with size of the file
  buf = calloc(1, file_size+1);
  if (!buf) {
    fclose(fp);
    perror("webserver (ReadFile: buffer calloc)");
    return "BAD FILE READ";
  }

  // Copy file contents into the buffer
  if (fread(buf, file_size, 1, fp) != 1) {
    fclose(fp);
    free(buf);
    perror("webserver (ReadFile: fread())");
    return "BAD FILE READ";
  }
  fclose(fp);
  return buf;
}

// Prints a string in the format of how 
// we want things to be logged
void
PrintLog(char* text) {
  char* time = FormatTime();
  printf("%s::%s\n",time, text);
  fflush(stdout);

  free(time);
  return;
}

// Parses the input HTTP request and
// gets the method, version, and URI
request_t*
ParseRequest(char* request) {
  request_t* req = (request_t*)malloc(sizeof(request_t));

  req->Method = (char*)malloc(BUFFER_SIZE);
  req->URI = (char*)malloc(BUFFER_SIZE);
  req->Version = (char*)malloc(BUFFER_SIZE);

  // Read the request
  sscanf(request, "%s %s %s", req->Method, req->URI, req->Version);

  return req;
}

void*
ThreadFunction(void* args) {
  return NULL;
}

int
main() {
  // Create a socket
  int socketfd = CreateSocket();
  if (socketfd < 0) {
    exit(1);
  } else {
    PrintLog("socket created successfully");
    fflush(stdout);
  }
  
  int listen = BindAndListen(socketfd, PORT);
  if (listen < 0) {
    perror("webserver (listen)");
    exit(1);
  }
  return 0;
}
