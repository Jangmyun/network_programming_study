#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define BUF_SIZE 1024
// #define DEBUG

void error_handling(char *message);
int count_file(DIR *directory);

int main(int argc, char *argv[]) {
  int server_socket = 0;
  int client_socket = 0;
  char buf[BUF_SIZE];
  int str_len;

  DIR *directory = NULL;
  struct dirent *entry = NULL;
  struct stat st;

  int currBuffSize = 0;

  directory = opendir(".");
  if (directory == NULL) {
    perror("opendir() failed");
    exit(1);
  }

  struct sockaddr_in serv_addr, clnt_addr;
  socklen_t clnt_addr_size;

  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }
}

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}