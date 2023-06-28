#pragma once
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "http.h"

// Gets the time and formats it properly to use
// with the log
char*
FormatTime() {
  char* output;
  time_t rawtime;
  struct tm* timeinfo;
  
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  output = calloc(60, sizeof(char)); // 20 bytes for now
  sprintf(
    output, 
    "[%d-%d-%d | %d:%d:%d]", 
    timeinfo->tm_mday, 
    timeinfo->tm_mon+1, 
    timeinfo->tm_year+1900, 
    timeinfo->tm_hour, 
    timeinfo->tm_min, 
    timeinfo->tm_sec
  );
  return output;
}


// Prints a string in the format of how 
// we want things to be logged
void
PrintLog(char* text) {
  char* time = FormatTime();
  printf("%s::%s\n",time, text);
  fflush(stdout);

  free(time);
  return;
}
