#pragma once
#ifndef HTTP_UTILS_
#define HTTP_UTILS_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct HTTP_Request {
  char *Method;
  char *URI;
  char *Version;
} request_t;

extern request_t* new_http_request(const char* req); // returns pointer to new request after parsing
extern void free_http_request(request_t* req); // frees a given HTTP_Request struct and its members


#endif // HTTP_UTILS_
