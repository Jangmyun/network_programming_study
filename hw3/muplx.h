#ifndef MUPLX_H
#define MUPLX_H

#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_FILENAME_SIZE 256

typedef struct {
  char filename[MAX_FILENAME_SIZE + 1];
  off_t size;
  int isDir;
} FileInfo;

void errorExit(const char *err);

ssize_t readn(int fd, void *data, size_t n);
ssize_t writen(int fd, const void *data, size_t n);

int count_files(char *path);
FileInfo *readFiles(char *path, int *fileCount);

void sendCwdInfos(int clnt_sock);
void receiveCwdInfos(int serv_sock);

int receiveResponse(int sock);

#endif