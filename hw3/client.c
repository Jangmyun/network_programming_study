#include "muplx.h"

int receiveResponse(int sock);
void cdResHandler(int sock);
void downloadResHandler(int sock, char *filename);
void uploadResHandler(int sock, char *filename);

int main(int argc, char *argv[]) {
  int sock = 0;
  char buf[BUF_SIZE];
  int rw_len;
  struct sockaddr_in serv_addr;

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
    errorExit("connect() error");
  }

  // 서버의 현재 디렉토리 정보 출력
  receiveCwdInfos(sock);

  // client는 계속해서 명령어 처리
  while (1) {
    puts("<Available Command List>");
    puts("ls, cd <dirname>, download <filename>, upload <filename>, quit");
    printf("Enter input > ");
    fgets(buf, BUF_SIZE, stdin);
    buf[strcspn(buf, "\n")] = 0;

#ifdef DEBUG
    printf("\"%s\"\n", buf);
#endif

    size_t commandSize = strlen(buf) + 1;

    writen(sock, &commandSize, sizeof(commandSize));
    writen(sock, buf, commandSize);

#ifdef DEBUG
    printf("Sent command: \"%s\" (size: %ld)\n", buf, commandSize);
#endif

    // 입력받은 command 파싱
    char *command = strtok(buf, " ");

    // q면 quit 메시지 보내고 반복문 종료
    if (!strcmp(command, "quit")) {
      writen(sock, "quit", 5);
      puts("Bye!");
      break;
    }

    // command의 argument 파싱
    char *commandArg = strtok(NULL, " ");
    if (commandArg == NULL) continue;

    printf("Command:%s Arg:%s\n", command, commandArg);

    if (!strcmp(command, "ls")) {
      receiveCwdInfos(sock);
    } else if (!strcmp(command, "cd")) {
      cdResHandler(sock);
    } else if (!strcmp(command, "download")) {
      downloadResHandler(sock, commandArg);
    } else if (!strcmp(command, "upload")) {
    } else {
      printf("Command not found : %s\n", command);
      continue;
    }
  }

  close(sock);
  return 0;
}

int receiveResponse(int sock) {
  size_t responseSize;
  readn(sock, &responseSize, sizeof(responseSize));

  if (responseSize == 0) {
    return 0;
  }

  char buf[responseSize];
  readn(sock, buf, responseSize);
  buf[responseSize - 1] = '\0';

  fputs(buf, stderr);
  return -1;
}

void cdResHandler(int sock) {
  if (receiveResponse(sock) == -1) {
    fputs("cd failed", stderr);
    return;
  }

  receiveCwdInfos(sock);
}

void downloadResHandler(int sock, char *filename) {
  if (receiveResponse(sock) == -1) {
    fputs("fopen() failed", stderr);
    return;
  }

  size_t fileSize;
  readn(sock, &fileSize, sizeof(fileSize));

  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    perror("fopen() failed");
    return;
  }

  char buf[BUF_SIZE];
  size_t leftBytes = fileSize;
  int read_count = 0;
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
  printf("Download [%s] Success\n", filename);
}

void uploadResHandler(int sock, char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    char *errorMessage = "fopen() failed";
    size_t errorMessageLen = strlen(errorMessage) + 1;
    writen(sock, &errorMessageLen, sizeof(errorMessageLen));
    writen(sock, errorMessage, errorMessageLen);
    return;
  }

  size_t ok = 0;
  writen(sock, &ok, sizeof(ok));  // 성공 메시지

  fseek(fp, 0, SEEK_END);
  size_t fileSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);  // 또는 rewind(fp);

  writen(sock, &fileSize, sizeof(fileSize));  // 파일 크기 전송

  char buf[BUF_SIZE];
  int read_count;
  while (1) {
    read_count = fread((void *)buf, 1, BUF_SIZE, fp);
    if (read_count < BUF_SIZE) {
      write(sock, buf, read_count);
      break;
    }
    write(sock, buf, BUF_SIZE);
  }

  fclose(fp);
}
