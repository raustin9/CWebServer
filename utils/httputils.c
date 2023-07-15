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

  res->String_Size = 0;

  res->Headers.Status = NULL;
  res->Headers.Content_Length = NULL;
  res->Headers.Content_Type = NULL;

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
  free(res->Headers.Content_Type);

  free(res->Body.Data);

  free(res);
}

// Create the body of the response
// from the desired data
  body_t
create_response_body(const unsigned char *data, size_t size)
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
set_http_response_body(response_t *res, const unsigned char *data, size_t size)
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
// return 1 if successful
// return -1 on error
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
  } else if (strcmp(header, "Content-Type") == 0) {
    res->Headers.Content_Type = strdup(data);
    result = 1;
  }
  else {
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

  response_string = (char*)calloc(1024+res->Body.Data_Size, sizeof(char));
  headers_length = sprintf(
    response_string,
    "HTTP/1.0 %s\r\n"
    "Server: c-webserver\r\n"
    "Connection: close\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %s\r\n\r\n",
    res->Headers.Status,
    res->Headers.Content_Type,
    res->Headers.Content_Length
  );

  // Use memcpy to account for null terminators
  // inside the body
  memcpy(response_string+headers_length, res->Body.Data, res->Body.Data_Size);
  res->String_Size = headers_length + res->Body.Data_Size;

  return response_string;
}

// Get what kind of content type (MIME) the HTTP response
// should be based on file extension
// ex. index.html  -> text/html
//     config.json -> application/json
char*
get_content_type(char* file_name) {
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

// Validate the request URI
// Return 1 if they are requesting "/"
// Return 2 if they are requesting an API call
// Return 3 if they are requesting a file
int
validate_uri(request_t *req)
{
  int result;

  if (strcmp(req->URI, "/") == 0) {
    // Assume it is browser asking for index.html
    result = 1;
  } else if (strcmp(req->URI, "/api") == 0) {
    // API call rather than file request
    result = 2;
  } else {
    // Assume it is file request
    result = 3;
  }

  return result;
}
