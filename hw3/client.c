#include "muplx.h"

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
    puts("ls, cd <dirname>, download <filename>, upload <filename>");
    printf("Enter input (q for quit) > ");
    fgets(buf, BUF_SIZE, stdin);

    // 입력받은 command 파싱
    buf[strlen(buf) - 1] = '\0';
    char *command = strtok(buf, " ");

    // q면 quit 메시지 보내고 반복문 종료
    if (!strcmp(command, "q")) {
      writen(sock, "quit", 5);
      puts("Bye!");
      break;
    }

    // command의 argument 파싱
    char *commandArg = strtok(NULL, " ");
    if (commandArg == NULL) continue;

#ifdef DEBUG
    printf("Command:%s | Arg:%s\n", command, arg);
#endif

    size_t commandSize = strlen(buf) + 1;
    writen(sock, &commandSize, sizeof(commandSize));
    writen(sock, buf, commandSize);

    if (!strcmp(command, "ls")) {
    } else if (!strcmp(command, "cd")) {
    } else if (!strcmp(command, "download")) {
    } else if (!strcmp(command, "upload")) {
    } else {
      printf("Command not found : %s\n", command);
      continue;
    }
  }

  close(sock);
  return 0;
}
