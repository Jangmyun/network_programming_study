#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_FILENAME_SIZE 259

typedef struct {
  char filename[MAX_FILENAME_SIZE + 1];
  off_t size;
} FileInfo;

int count_files(char *path);
FileInfo *read_files(char *path, int filecount);

#endif
