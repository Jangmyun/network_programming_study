#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
// #define DEBUG

void error_handling(char *message);

int main(int argc, char *argv[]) { return 0; }

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}