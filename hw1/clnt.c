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
ssize_t readn(int fd, void *data, size_t n);
ssize_t writen(int fd, const void *data, size_t n);

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

  // 다른 파일 계속해서 받을지 선택하며 반복
  while (1) {
    read(sock, &filecount, sizeof(int));
    if (filecount < 1) {
      printf("filecount invalid\n");
      close(sock);
      exit(1);
    }
#ifdef DEBUG
    printf("filecount = %d\n", filecount);
#endif

    FileInfo fileinfo;
    printf("\n<Read File Informations>\n");
    for (int i = 0; i < filecount; i++) {
      read_cnt = readn(sock, &fileinfo, sizeof(FileInfo));
#ifdef DEBUG
      printf("%d: %s | %ldbytes | FILEREAD:%d\n", i + 1, fileinfo.filename,
             fileinfo.size, read_cnt);
#endif
      printf("%d: %s | %ldbytes\n", i + 1, fileinfo.filename, fileinfo.size);
    }

    // 파일 번호 입력받기
    int file_selected = 0;
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
#ifdef DEBUG
    printf("file_selected=%d\n", file_selected);
#endif
    writen(sock, &file_selected, sizeof(int));

    // File 정보 받은 후 파일 받기
    readn(sock, &fileinfo, sizeof(FileInfo));

    FILE *fp = fopen(fileinfo.filename, "wb");
    if (fp == NULL) {
      perror("fopen() failed");
      break;
    }

    long left_bytes = fileinfo.size;
    while (left_bytes > 0) {
      if (left_bytes < BUF_SIZE) {
        read_cnt = readn(sock, buf, left_bytes);
        fwrite((void *)buf, 1, left_bytes, fp);
        left_bytes -= read_cnt;
        break;
      }

      read_cnt = readn(sock, buf, BUF_SIZE);
      fwrite((void *)buf, 1, read_cnt, fp);
      left_bytes -= read_cnt;
    }

    // 계속 반복할지 입력받기
    int cond = 1;
    fputs("Keep going? (y / n) > ", stdout);
    fgets(buf, BUF_SIZE, stdin);
    if (strcmp(buf, "y")) {
      printf("no iteration\n");
      cond = 0;
      writen(sock, &cond, sizeof(int));
      break;
    }
    printf("Keep go!\n");
    writen(sock, &cond, sizeof(int));
  }  // while

  printf("Bye!\n");

  close(sock);
  return 0;
}

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

ssize_t readn(int fd, void *data, size_t n) {
  size_t left;
  size_t read_n;

  left = n;

  while (left > 0) {
    if ((read_n = read(fd, data, left)) < 0) {
      if (left == n)
        return -1;
      else
        break;
    } else if (read_n == 0)
      break;
    left -= read_n;
    data += read_n;
  }

  return n - left;
}

ssize_t writen(int fd, const void *data, size_t n) {
  size_t left;
  ssize_t written_n;

  left = n;

  while (left > 0) {
    if ((written_n = write(fd, data, left)) < 0) {
      if (left == n)
        return -1;
      else
        break;
    } else if (written_n == 0)
      break;
    left -= written_n;
    data += written_n;
  }

  return n - left;
}