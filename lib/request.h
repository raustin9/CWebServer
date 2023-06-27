#pragma once

// Information from HTTP Request
typedef struct HTTPRequestData {
  char* Method;
  char* URI;
  char* Version;
} request_t;
