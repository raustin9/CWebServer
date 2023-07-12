#pragma once
#ifndef HTTP_UTILS_
#define HTTP_UTILS_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Structure for the headers of HTTP Response
typedef struct HTTP_Response_Headers {
  char *Status;
  char *Content_Length;
} headers_t;

// Structure for the body of HTTP Response
typedef struct HTTP_Response_Body {
  char *Data;
  size_t Data_Size;
} body_t;

// Structure of an HTTP Request
typedef struct HTTP_Request {
  char *Method;
  char *URI;
  char *Version;
} request_t;

// Structure of an HTTP Response
typedef struct HTTP_Response {
  headers_t Headers;
  body_t Body;
} response_t;


extern request_t* new_http_request(const char* req); // returns pointer to new request after parsing
extern void free_http_request(request_t* req); // frees a given HTTP_Request struct and its members
extern response_t* new_http_response();              // returns pointer to a new http response
extern void free_http_response(response_t *res); // frees a given HTTP_Request struct and its members
extern int set_http_response_header(response_t *res, const char *header, char *data);
extern void set_http_response_body(response_t *res, const char *data, size_t size);
extern char* create_http_response_string(response_t *res); // create the string to be sent from the data in the http response
#endif // HTTP_UTILS_
