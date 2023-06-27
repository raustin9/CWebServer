/*
 *  HTTPhelpers.c
 *
 *  this file contains some helper functions when crafting
 *  or parsing HTTP requests and responses
 */

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "request.h"

#define BUFFER_SIZE 1024
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
char*
CreateResponse(int status, char* resp_data, char* content_type) {
  char* rv;
  char* statusmsg;

  statusmsg = GetStatusMsg(status);

  rv = (char*)malloc(
    (strlen("HTTP/1.1 %s\r\nServer: webserver-c")
    + strlen(content_type)+1+4
    + strlen(resp_data)+1 
    + strlen(statusmsg)+1
    + 20) * sizeof(char)
  );

  sprintf(
    rv, 
    "HTTP/1.1 %s\r\n"
    "Server: webserver-c\r\n"
    "%s\r\n\r\n"
    "%s\r\n",
    statusmsg,
    content_type,
    resp_data
  );

  free(statusmsg); 
  return rv;
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
