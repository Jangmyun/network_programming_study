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

void errorExit(const char *err);

ssize_t readn(int fd, void *data, size_t n);
ssize_t writen(int fd, const void *data, size_t n);

#endif