#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define BUF_SIZE 1024

void error_handling(char *message);
int count_files(char *path);

int main(int argc, char *argv[]) {
  int server_socket = 0;
  int client_socket = 0;
  char buf[BUF_SIZE];
  int str_len = 0;

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

// path 디렉토리의 파일 개수를 카운트하고, filename_size set
int count_files(char *path) {
  DIR *dir = NULL;
  struct dirent *entry;

  if (path == NULL) return -1;
  if ((dir = opendir(path)) == NULL) return -1;

  int count = 0;

  while ((entry = readdir(dir))) {
    // file이면 count++
    if (entry->d_type == DT_REG) {
      count++;
    }
  }

  closedir(dir);

  return count;
}
