#pragma once
#ifndef _HTTPUTILS
#define _HTTPUTILS

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct HTTP_Request {
  char *Method;
  char *URI;
  char *Version;
};

HTTP_Request* new_http_request(char* req); // returns pointer to new request after parsing
void free_http_request(HTTP_Request* req); // frees a given HTTP_Request struct and its members


#endif // _HTTPUTILS
