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
#define MAX_FILENAME_SIZE 259

typedef struct {
  char filename[MAX_FILENAME_SIZE + 1];
  off_t size;
} FileInfo;

void error_handling(char *message);
int count_files(char *path);
FileInfo *read_files(char *path, int filecount);
ssize_t readn(int fd, void *data, size_t n);
ssize_t writen(int fd, const void *data, size_t n);

int main(int argc, char *argv[]) {
  int serv_sock = 0, clnt_sock = 0;
  char buf[BUF_SIZE];
  int read_count = 0;
  int filename_size = 0;
  int currBuffSize = 0;

  struct sockaddr_in serv_addr, clnt_addr;
  socklen_t clnt_addr_size;

  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int filecount = 0;

  serv_sock = socket(PF_INET, SOCK_STREAM, 0);

  int optval = 1;
  if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                 sizeof(optval))) {
    perror("setsockopt() error");
    exit(1);
  }

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
  printf("Server On\n");

  clnt_addr_size = sizeof(clnt_addr);

  // iterate file transfer
  while (1) {
    clnt_sock =
        accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
      continue;
    else
      printf("Connected client\n");

    while (1) {
      FileInfo *fileinfos = NULL;
      fileinfos = read_files(".", (filecount = count_files(".")));
#ifdef DEBUG
      printf("filecount = %d\n", filecount);
#endif
      writen(clnt_sock, &filecount, sizeof(int));

      for (int i = 0; i < filecount; i++) {
        writen(clnt_sock, &fileinfos[i], sizeof(FileInfo));
#ifdef DEBUG
        printf("SEND FILEINFO: filename=%s, size=%ld\n", fileinfos[i].filename,
               fileinfos[i].size);
#endif
      }

      // 파일 번호 수신
      int file_selected = 0;
      readn(clnt_sock, &file_selected, sizeof(int));
#ifdef DEBUG
      printf("file_selected = %d\n", file_selected);
#endif

      // 파일 송신
      writen(clnt_sock, &fileinfos[file_selected], sizeof(FileInfo));

      FILE *fp = fopen(fileinfos[file_selected].filename, "rb");
      if (fp == NULL) {
        perror("fopen() failed");
        free(fileinfos);
        break;
      }

      while (1) {
        read_count = fread((void *)buf, 1, BUF_SIZE, fp);
        if (read_count < BUF_SIZE) {
          write(clnt_sock, buf, read_count);
          break;
        }
        write(clnt_sock, buf, BUF_SIZE);
      }

      // 계속 반복할지 수신
      int cond = 0;
      readn(clnt_sock, &cond, sizeof(int));
      if (!cond) {
        break;
      } else {
        printf("Keep go!\n");
      }

      free(fileinfos);

    }  // while

    printf("Bye!\n");
    close(clnt_sock);

  }  // while

  close(serv_sock);
  return 0;
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

// count된 파일 개수만큼 stat 구조체 배열 동적할당하여 포인터 리턴
FileInfo *read_files(char *path, int filecount) {
  int count = 0;
  if (path == NULL || filecount < 1) exit(1);

  FileInfo *fileinfos = (FileInfo *)malloc(sizeof(FileInfo) * filecount);
  if (fileinfos == NULL) {
    perror("malloc() failed");
    exit(1);
  }

  DIR *dir = NULL;
  struct dirent *entry;
  struct stat curr_stat;

  dir = opendir(path);
  if (dir == NULL) {
    perror("opendir() failed");
    exit(1);
  }

  while ((entry = readdir(dir))) {
    if (count == filecount) break;
    if (entry->d_type == DT_REG) {
      stat(entry->d_name, &curr_stat);
      strncpy(fileinfos[count].filename, entry->d_name, MAX_FILENAME_SIZE);
      fileinfos[count].filename[MAX_FILENAME_SIZE] = 0;
      fileinfos[count].size = curr_stat.st_size;

      count++;
    }
  }

  closedir(dir);
  return fileinfos;
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