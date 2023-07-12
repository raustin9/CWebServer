#include "httputils.h"

// Parse http request and fill in the values
// @param request {char*} -- string of HTTP request to parse
request_t*
new_http_request(const char *request)
{
  request_t *req = (request_t*)calloc(1, sizeof(request_t));
  int t;

  req->Method = (char*)calloc(1024, sizeof(char));
  req->Version = (char*)calloc(1024, sizeof(char));
  req->URI = (char*)calloc(1024, sizeof(char));

  strcpy(req->Method, "INVALID");
  strcpy(req->URI, "INVALID");
  strcpy(req->Version, "INVALID");

  t = sscanf(request, "%s %s %s", req->Method, req->URI, req->Version);
  if (t != 3)
  {
    perror("new_http_request");
    return NULL;
  }

  return req;
}

// Free the members of the request and the request itself
void
free_http_request(request_t *req)
{
  free(req->Method);
  free(req->URI);
  free(req->Version);
  free(req);
}

// Create a new HTTP response
response_t*
new_http_response()
{
  response_t* res;
  res = (response_t*)calloc(1, sizeof(response_t));

  res->Headers.Status = NULL;
  res->Headers.Content_Length = NULL;

  res->Body.Data = NULL;
  res->Body.Data_Size = 0;

  return res;
}

// Free members of the response 
// AND the response itself
void
free_http_response(response_t *res)
{
  free(res->Headers.Status);
  free(res->Headers.Content_Length);

  free(res->Body.Data);

  free(res);
}

// Create the body of the response
// from the desired data
  body_t
create_response_body(const char *data, size_t size)
{
  body_t b;

  b.Data = (char*)calloc(size, sizeof(char));
  b.Data_Size = size;

  // Copy the memory into the data section
  // to account for null terminator in 
  // non-text files
  memcpy(b.Data, (void*)data, size);

  return b;
}

// Set the body of http response
// to desired data
void
set_http_response_body(response_t *res, const char *data, size_t size)
{
  body_t body;
  char content_length[32];

  body = create_response_body(data, size);
  res->Body = body;

  if (sprintf(content_length, "%zu", size) < 0)
  {
    perror("set_http_response_body (sprintf)");
    if (res->Headers.Content_Length != NULL) {
      free(res->Headers.Content_Length);
      res->Headers.Content_Length = NULL;
    } else {
      res->Headers.Content_Length = NULL;
    }
    return;
  }

  set_http_response_header(res, "Content-Length", content_length);

  return;
}


// Set the header of an HTTP response
// to the desired data
int
set_http_response_header(response_t *res, const char *header, char *data)
{
  int result;

  if (strcmp(header, "Status") == 0) {
    res->Headers.Status = strdup(data);
    result = 1;
  } else if (strcmp(header, "Content-Length") == 0) {
    res->Headers.Content_Length = strdup(data);
    result = 1;
  } else {
    result = -1;
  }

  return result;
}

// Create the string to be sent
// from the fields in the response
// struct
char*
create_http_response_string(response_t *res)
{
  char *response_string;
  long int headers_length;

  response_string = (char*)calloc(1024, sizeof(char));
  headers_length = sprintf(
    response_string,
    "HTTP/1.1 %s\r\n"
    "Server: c-webserver\r\n"
    "Content-Length: %s\r\n\r\n",
    res->Headers.Status,
    res->Headers.Content_Length
  );

  // Use memcpy to account for null terminators
  // inside the body
  memcpy(response_string+headers_length, res->Body.Data, res->Body.Data_Size);

  return response_string;
}
