#pragma once
#ifndef FILE_UTILS_
#define FILE_UTILS_

#include <stdlib.h>

typedef struct File {
  char *Name;  // name of the file
  unsigned char *Data;  // the content of the file in bytes
  size_t Size;
} file_t;

extern char* get_file_name(char *URI);  // parse the name of file from URI
extern file_t* read_file(char *file_name); // read data from file into file struct
extern void free_file(file_t *file); // free the file struct
extern char* create_file_path(char *dirname, char *file_name); // create the path too a file
#endif /* FILE_UTILS_ */
