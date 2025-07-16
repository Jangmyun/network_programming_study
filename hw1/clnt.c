#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define MAX_FILENAME_SIZE 259

typedef struct {
  char filename[MAX_FILENAME_SIZE + 1];
  off_t size;
} FileInfo;

void error_handling(char *message);

int main(int argc, char *argv[]) {
  int sock = 0;
  char buf[BUF_SIZE];
  int read_cnt = 0;
  struct sockaddr_in serv_addr;

  int filecount = 0;

  if (argc != 3) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  sock = socket(PF_INET, SOCK_STREAM, 0);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    perror("connect() error");
    exit(1);
  }

  read(sock, &filecount, sizeof(int));
  if (filecount < 1) {
    printf("filecount invalid\n");
    close(sock);
    exit(1);
  }
#ifdef DEBUG
  printf("filecount = %d\n", filecount);
#endif

  int file_selected = 0;
  long file_size = 0;
  FileInfo fileinfo;
  printf("\n<Read File Informations>\n");
  for (int i = 0; i < filecount; i++) {
    read(sock, &fileinfo, sizeof(FileInfo));
    printf("%d: %s | %ldbytes\n", i + 1, fileinfo.filename, fileinfo.size);
  }

  while (1) {
    fputs("Select file number to receive > ", stdout);
    fgets(buf, BUF_SIZE, stdin);
    file_selected = atoi(buf);
    if (file_selected > 0) {
      break;
    }
    fputs("Input invalid.\n", stdout);
  }
  file_selected--;
  write(sock, &file_selected, sizeof(int));

  close(sock);
  return 0;
}

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}