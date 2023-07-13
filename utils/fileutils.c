#include <fileutils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// This string takes in the uri from an http request
// and strips the file name from it
//  -- "/index.html" -> "index.html"
char*
get_file_name(char *uri)
{
  return strdup(uri+1);
}

// This function reads the bytes from the 
// desired file and returns a file-t struct
// filled with the needed information
file_t*
read_file(char* file_name)
{
  file_t* file;
  FILE *fp;
  size_t file_size;

  struct stat st;

  // Initialze the file struct
  file = (file_t*)calloc(1, sizeof(file_t));
  file->Size = 0;
  file->Data = NULL;

  // Check if file exists
  if (access(file_name, F_OK) != 0)
  {
    perror("read_file (access)");
    file->Data = NULL;
    file->Size = 0;
    return file;
  }


  // Open file
  fp = fopen(file_name, "rb");
  if (fp == NULL)
  {
    fclose(fp); // do I need this?
    perror("read_file (fopen)");
    file->Data = NULL;
    file->Size = 0;
    return file;
  }

  stat(file_name, &st);
  file_size = st.st_size;

//  // Seek to end of file and read how far in
//  // to get number of bytes to read
//  fseek(fp, 0L, SEEK_END);
//  file_size = ftell(fp);
//  if (file_size < 0)
//  {
//    fclose(fp);
//    perror("read_file (ftell)");
//    file->Data = NULL;
//    file->Size = 0;
//    return file;
//  }
//  rewind(fp);
  fseek(fp, 0L, SEEK_SET);
  
  file->Size = file_size;

  // Allocate space in struct 
  // to read from the file
  file->Data = (char*)calloc(file_size, sizeof(char));
  if (file->Data == NULL) 
  {
    fclose(fp);
    perror("read_file (calloc)");
    file->Data = NULL;
    file->Size = 0;
    return file;
  }

  // Copy file content into the file struct
  if (fread(file->Data, file_size, 1, fp) != 1)
  {
    fclose(fp);
    perror("read_file (fread)");
    file->Data = NULL;
    file->Size = 0;
    return file;
  }

  fclose(fp);
  return file;
}

// Free the information in the file
void
free_file(file_t *file)
{
  free(file->Data);
  free(file);
}

// Creates the file path for a specified file
// -- path: files/
// -- name: index.html
// --> files/index.html
char*
create_file_path(char* dirname, char* file_name)
{
  char* full_path;

  full_path = (char*)calloc(
    strlen(dirname) +
    strlen("/") +
    strlen(file_name) +
    1,
    sizeof(char)
  );

  memcpy(full_path, dirname, strlen(dirname));
  memset(full_path+strlen(dirname), '/', 1);
  memcpy(full_path+strlen(dirname)+strlen("/"), file_name, strlen(file_name));
  memset(full_path+strlen(dirname)+strlen("/")+strlen(file_name), '\0', 1);

  return full_path;
}
