/*
 *  HTTPhelpers.c
 *
 *  this file contains some helper functions when crafting
 *  or parsing HTTP requests and responses
 */

#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// #include "request.h"


// Information from HTTP Request
typedef struct HTTPRequestData {
  char* Method;
  char* URI;
  char* Version;
} request_t;


#define BUFFER_SIZE 1024

void
FreeRequest(request_t* req) {
  free(req->Method);
  free(req->URI);
  free(req->Version);
}

// Gets the status message based on the status code
// ex: 200 => "200 OK"
char*
GetStatusMsg(int status) {
  char* rv;
  rv = (char*)malloc(100);

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

    case 404:
      strcpy(rv, "404 Not Found");
      break;

    default:
      strcpy(rv, "404 Not Found");
      break;
  }

  return rv;
}


// Generetes and HTTP response based on the status code and desired response body
// Params:
//  status       -- status code for HTTP response: 200 OK or 404 NOT FOUND
//  resp_data    -- body data for the http response
//  content_type -- content type header for response: text/html or application/json
//  body_size    -- amount of bytes for the response body
char*
CreateResponse(int status, char* resp_data, char* content_type, uint64_t body_size) {
  char *headers, *body, *content_len;
  char* statusmsg;
  uint64_t header_size;

  content_len = (char*)calloc(30, sizeof(char));
  sprintf(content_len, "%lu", body_size);

  statusmsg = GetStatusMsg(status);
  header_size = (
      strlen("HTTP/1.1 %s\r\nServer: webserver-c\r\nContent-Length: \r\nContent-Type: \r\n")
    + strlen(content_type)+1+4
    + body_size+1 
    + strlen(statusmsg)+1
    + strlen(content_len)
    + 20
  );

  headers  = (char*)calloc(
    sizeof(char),
    header_size
  );

  body = (char*)calloc(
    sizeof(char),
    header_size
  );

  uint64_t length = sprintf(
    headers,
    "HTTP/1.1 %s\r\n"
    "Server: webserver-c\r\n"
    "Content-Length: %s"
    "Content-Type: %s\r\n\r\n",
    statusmsg,
    content_len,
    content_type
  );

  // memcpy magic for files that have 
  // null terminators in them like images
  memcpy(body, headers, length);
  memcpy(body+length, resp_data, body_size);
//  printf("headers:\n%s\n", headers);
//  printf("body: \n%s\n", body);
 
  free(headers);
  free(content_len);
  free(statusmsg); 
  return body;
}


// Parses the input HTTP request and
// gets the method, version, and URI
request_t*
ParseRequest(const char* request) {
  request_t* req = (request_t*)malloc(sizeof(request_t));

  req->Method = (char*)malloc(BUFFER_SIZE);
  req->URI = (char*)malloc(BUFFER_SIZE);
  req->Version = (char*)malloc(BUFFER_SIZE);

  strcpy(req->Method, "INVALID");
  strcpy(req->URI, "INVALID");
  strcpy(req->Version, "INVALID");

  // Read the request
  if (sscanf(request, "%s %s %s", req->Method, req->URI, req->Version) != 3) {
    printf("FUCKME\n");
    fflush(stdout);
  }
  // free(request);
  return req;
}

// Parse the URI from the request
// FOR NOW: browser will request files, so assume all uri information is file names
// FUTURE:  add some form of REST API, so check if uri is requesting '/api' or something
char* ParseURI(char* req_uri) {
  return strdup(req_uri+1); // '/app.css' -> 'app.css'
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
    content_type = strdup("text/plain");
  }

  free(file_extension);
  return content_type;
}
