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
