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
  int serv_sock = 0, clnt_sock = 0;
  char buf[BUF_SIZE];
  int str_len = 0;
  int filename_size = 0;
  int currBuffSize = 0;

  struct sockaddr_in serv_addr, clnt_addr;
  socklen_t clnt_addr_size;

  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int filecount = count_files(".");

#ifdef DEBUG
  printf("filecount=%d, filename_size=%d\n", filecount, filename_size);
#endif

  serv_sock = socket(PF_INET, SOCK_STREAM, 0);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    perror("bind() error");
  }

  if (listen(serv_sock, 5) == -1) {
    perror("listen() error");
  }

  clnt_addr_size = sizeof(clnt_addr);
  clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
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
