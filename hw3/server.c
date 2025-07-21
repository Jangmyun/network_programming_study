#include <errno.h>
#include <pthread.h>

#include "muplx.h"

void *serverThreadFunction(void *arg);

void cdHandler(int sock, char *dest);
void downloadHandler(int sock, char *filename);
void uploadHandler(int sock, char *filename);

int main(int argc, char *argv[]) {
  int serv_sock, clnt_sock;
  struct sockaddr_in serv_addr, clnt_addr;

  struct timeval timeout;
  const struct timeval TIMEOUT = {5, 5000};

  socklen_t addr_size;

  int fd_max, fd_num;
  int rw_len;
  char buf[BUF_SIZE];

  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  int optval = 1;
  if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                 sizeof(optval))) {
    errorExit("setsockopt() error");
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    errorExit("bind() error");
  }

  if (listen(serv_sock, 5) == -1) {
    errorExit("listen() error");
  }

  fd_set reads, tempReads;
  fd_set writes, tempWrites;

  FD_ZERO(&reads);
  FD_SET(serv_sock, &reads);
  fd_max = serv_sock;

  puts("-Server On-");

  pthread_t tid;
  while (1) {
    tempReads = reads;

    timeout = TIMEOUT;

    fd_num = select(fd_max + 1, &tempReads, 0, 0, &timeout);
    if (fd_num == -1) break;
    if (fd_num == 0) continue;

    for (int i = 0; i <= fd_max; i++) {
#ifdef DEBUG
      printf("check:%dth sd\n", i);
#endif
      // i번째 가 readfds에 속했을 때
      if (FD_ISSET(i, &tempReads)) {
        // 서버 소켓으로 connect 요청 시 수락하고 cwd 정보 전송
        if (i == serv_sock) {
          addr_size = sizeof(clnt_addr);
          clnt_sock =
              accept(serv_sock, (struct sockaddr *)&clnt_addr, &addr_size);
          FD_SET(clnt_sock, &reads);
          if (fd_max < clnt_sock) fd_max = clnt_sock;
          printf("Client connected: %d\n", clnt_sock);

          int ret = pthread_create(&tid, NULL, serverThreadFunction,
                                   (void *)&clnt_sock);
          if (ret != 0) {
            fprintf(stderr, "pthread_create() failed %s\n", strerror(errno));
          }

          pthread_detach(tid);
        }
      }
    }
  }

  return 0;
}

void *serverThreadFunction(void *arg) {
  int sock = *(int *)arg;
  char buf[BUF_SIZE];

  size_t commandSize;
  int rw_len;

  sendCwdInfos(sock);

  while (1) {
    rw_len = readn(sock, &commandSize, sizeof(commandSize));
    if (rw_len <= 0) {
      fprintf(stderr, "Connection Lost (%d)\n", sock);
      break;
    }

    rw_len = readn(sock, buf, commandSize);
    if (rw_len <= 0) {
      fprintf(stderr, "Connection Lost (%d)\n", sock);
      break;
    }

    buf[commandSize - 1] = '\0';

#ifdef DEBUG
    printf("Received command size: %ld\n", commandSize);
    printf("Received command: \"%s\"\n", buf);
#endif

    char *command = strtok(buf, " ");

    if (!strcmp(command, "quit")) {
      printf("Client %d quit\n", sock);
      break;
    }
    if (!strcmp(command, "ls")) {
      sendCwdInfos(sock);
      continue;
    }

    char *commandArg = strtok(NULL, " ");
    if (commandArg == NULL) continue;

#ifdef DEBUG
    printf("Sock:%d, Command:%s, Arg:%s\n", sock, command, commandArg);
#endif

    if (!strcmp(command, "cd")) {
      cdHandler(sock, commandArg);
    } else if (!strcmp(command, "download")) {
      downloadHandler(sock, commandArg);
    } else if (!strcmp(command, "upload")) {
      uploadHandler(sock, commandArg);
    } else {
      printf("Command not found : %s (%d)\n", command, sock);
      continue;
    }

  }  // while

  close(sock);
  return NULL;
}

void cdHandler(int sock, char *dest) {
  if (chdir(dest) == -1) {
    char *errorMessage = "chdir() failed";
    size_t errorMessageLen = strlen(errorMessage) + 1;
    writen(sock, &errorMessageLen, sizeof(errorMessageLen));
    writen(sock, errorMessage, errorMessageLen);
    return;
  }
  fputs("cd success", stdout);
  size_t ok = 0;
  writen(sock, &ok, sizeof(ok));

  sendCwdInfos(sock);
}

void downloadHandler(int sock, char *filename) {
#ifdef DEBUG
  printf("Send %s\n", filename);
#endif
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
#ifdef DEBUG
    printf("fopen() failed\n");
#endif
    char *errorMessage = "fopen() failed";
    size_t errorMessageLen = strlen(errorMessage) + 1;
    writen(sock, &errorMessageLen, sizeof(errorMessageLen));
    writen(sock, errorMessage, errorMessageLen);
    return;
  }

#ifdef DEBUG
  printf("fopen() success\n");
#endif

  size_t ok = 0;
  writen(sock, &ok, sizeof(ok));

  fseek(fp, 0, SEEK_END);
  size_t fileSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  printf("Sending %s ...\n", filename);

  writen(sock, &fileSize, sizeof(fileSize));

  char buf[BUF_SIZE];
  int read_count = 0;
  while (1) {
    read_count = fread((void *)buf, 1, BUF_SIZE, fp);
    if (read_count < BUF_SIZE) {
      write(sock, buf, read_count);
      break;
    }
    write(sock, buf, BUF_SIZE);
  }

  fclose(fp);
  printf("Uploading [%s] Success\n", filename);
}

void uploadHandler(int sock, char *filename) {
  if (receiveResponse(sock) == -1) {
    fputs("client fopen() failed", stderr);
    return;
  }

  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    perror("fopen() failed");
    return;
  }

  size_t fileSize;
  readn(sock, &fileSize, sizeof(fileSize));

  char buf[BUF_SIZE];
  size_t leftBytes = fileSize;
  int read_count;
  while (leftBytes > 0) {
    if (leftBytes < BUF_SIZE) {
      read_count = readn(sock, buf, leftBytes);
      fwrite((void *)buf, 1, leftBytes, fp);
      leftBytes -= read_count;
      break;
    }

    read_count = readn(sock, buf, BUF_SIZE);
    fwrite((void *)buf, 1, read_count, fp);
    leftBytes -= read_count;
  }

  fclose(fp);
  printf("Downloading [%s] Success\n", filename);
}
