#pragma once
#ifndef FILE_UTILS_
#define FILE_UTILS_

#include <stdlib.h>

typedef struct File {
  char *Data;
  size_t Size;
} file_t;

extern char* get_file_name(char *URI);
extern file_t* read_file(char *file_name);
extern void free_file(file_t *file);

#endif /* FILE_UTILS_ */
