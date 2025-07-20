#include "muplx.h"

void cdHandler(int sock, char *dest);

int main(int argc, char *argv[]) {
  int serv_sock, clnt_sock;
  struct sockaddr_in serv_addr, clnt_addr;

  struct timeval timeout;
  const struct timeval TIMEOUT = {1, 0};

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

  while (1) {
    tempReads = reads;

    timeout = TIMEOUT;

    fd_num = select(fd_max + 1, &tempReads, &tempWrites, 0, &timeout);
    if (fd_num == -1) break;
    if (fd_num == 0) continue;

    for (int i = 0; i <= fd_max; i++) {
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

          sendCwdInfos(clnt_sock);
        } else {
          size_t commandSize;
          rw_len = readn(i, &commandSize, sizeof(commandSize));
          if (rw_len == 0) {
            fprintf(stderr, "Invalid Command\n");
            close(i);
            FD_CLR(i, &reads);
            FD_CLR(i, &writes);
            continue;
          }

          readn(i, buf, commandSize);

          buf[strlen(buf) - 1] = '\0';
          char *command = strtok(buf, " ");

          if (!strcmp(command, "q")) {
            close(i);
            FD_CLR(i, &reads);
            FD_CLR(i, &writes);
            continue;
          }

          char *commandArg = strtok(NULL, " ");
          if (commandArg == NULL) continue;

#ifdef DEBUG
          printf("Command:%s | Arg:%s\n", command, commandArg);
#endif

          if (!strcmp(command, "ls")) {
            sendCwdInfos(i);
          } else if (!strcmp(command, "cd")) {
            cdHandler(i, commandArg);
          } else if (!strcmp(command, "download")) {
          } else if (!strcmp(command, "upload")) {
          } else {
            printf("Command not found : %s\n", command);
            continue;
          }
        }
      }
    }
  }

  return 0;
}

void cdHandler(int sock, char *dest) {
  if (chdir(dest) == -1) {
    char *errorMessage = "chdir() failed";
    size_t errorMessageLen = strlen(errorMessage) + 1;
    writen(sock, &errorMessageLen, sizeof(errorMessageLen));
    writen(sock, errorMessage, errorMessageLen);
    return;
  }
  size_t ok = 0;
  writen(sock, &ok, sizeof(ok));

  sendCwdInfos(sock);
}
