#include "console.h"
#include "game.h"

#define MULTICAST_GROUP_IP "225.192.0.10"
#define TTL 20

board_pos *boardPositions = NULL;
GameInitInfo gameInitInfo;
board_bitarray board_pos_bitarray;  // 어떤 위치가 board인지
board_bitarray board_status;        // board들의 상태 (blue, red);

typedef struct _RecvThreadArg {
  int sock;
  int playerId;
} RecvThreadArg;

int currentPlayerId = 0;

int setOptions(int argc, char *argv[], int *playerCount, int *gridSize,
               int *boardCount, int *gameSec, int *port);
void *recvThreadFunc(void *arg);

int main(int argc, char *argv[]) {
  // default options
  int port = 5123;
  {
    int playerCount = 2;
    int gridSize = 32;
    int boardCount = 200;
    int gameSec = 60;

    setOptions(argc, argv, &playerCount, &gridSize, &boardCount, &gameSec,
               &port);

    struct timeval gameTime = {gameSec, 0};
    setGameInitInfo(&gameInitInfo, playerCount, currentPlayerId, gridSize,
                    boardCount, boardPositions, gameTime);

    if (port < 1024 || port > 65535) {
      printError("Cannot use well-known port");
      exit(1);
    }

    validateGameInfo(&gameInitInfo);
  }

#ifdef DEBUG
  puts("GameInitInfo Set");
  printGameInfo(&gameInitInfo);
  printf("PORT = %d\n", port);
#endif

  // 설정한 보드의 개수만큼 전체 grid 인덱스에서 보드의 위치를 랜덤하게 설정
  boardPositions = generateBoardPosition(&gameInitInfo, &board_pos_bitarray);

#ifdef DEBUG
  puts("Random Board Positions Generated");
  printBoardPositions(boardPositions, gameInitInfo.boardCount);
  puts("");
#endif

  // 선택한 보드들 중 랜덤한 절반의 flag를 1로 설정
  randomizeBoardColor(&board_status, gameInitInfo.boardCount);

#ifdef DEBUG
  puts("Random Board Color Selected");
  printfBoardStatus(&board_status, gameInitInfo.boardCount);
  puts("");
#endif

  int serv_tcp_sock, clnt_tcp_sock;
  struct sockaddr_in serv_tcp_addr, clnt_tcp_addr;
  socklen_t serv_tcp_addr_size, clnt_tcp_addr_size;

  int serv_udp_sock;
  struct sockaddr_in serv_udp_addr;  // multicast group addr

  serv_tcp_sock = socket(PF_INET, SOCK_STREAM, 0);
  serv_udp_sock = socket(PF_INET, SOCK_DGRAM, 0);

  // addr memset
  // TCP
  memset(&serv_tcp_addr, 0, sizeof(serv_tcp_addr));
  serv_tcp_addr.sin_family = AF_INET;
  serv_tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_tcp_addr.sin_port = htons(port);
  // UDP
  memset(&serv_udp_addr, 0, sizeof(serv_udp_addr));
  serv_udp_addr.sin_family = AF_INET;
  serv_udp_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP_IP);
  serv_udp_addr.sin_port = htons(port);  // TCP와 같은 포트

  // set socket options
  {
    // TCP REUSEADDR
    int optval = 1;
    if (setsockopt(serv_tcp_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                   sizeof(optval))) {
      perror("tcp setsockopt() (SO_REUSEADDR) error");
      close(serv_tcp_sock);
      exit(1);
    }
    // UDP REUSEADDR
    if (setsockopt(serv_udp_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                   sizeof(optval))) {
      perror("udp setsockopt() (SO_REUSEADDR) error");
      close(serv_udp_sock);
      exit(1);
    }
    // UDP TTL
    int ttl = 20;
    if (setsockopt(serv_udp_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl,
                   sizeof(ttl))) {
      perror("udp setsockopt() (IP_MULTICAST_TTL) error");
      close(serv_udp_sock);
      exit(1);
    }
  }

  if (bind(serv_tcp_sock, (struct sockaddr *)&serv_tcp_addr,
           sizeof(serv_tcp_addr)) == -1) {
    perror("tcp sock bind() error");
    exit(1);
  }

  if (listen(serv_tcp_sock, 8) == -1) {
    perror("tcp sock listen() error");
    exit(1);
  }

  // 모든 클라이언트가 접속하길 대기
  while (currentPlayerId < gameInitInfo.playerCount) {
    clnt_tcp_sock = accept(serv_tcp_sock, (struct sockaddr *)&clnt_tcp_addr,
                           &clnt_tcp_addr_size);
    if (clnt_tcp_sock == -1) continue;
    printf("Client accepted (sock=%d, id=%d)\n", clnt_tcp_sock,
           currentPlayerId);

    // thread function argument
    RecvThreadArg *t_arg = (RecvThreadArg *)malloc(sizeof(RecvThreadArg));
    if (t_arg == NULL) {  // malloc 실패처리
      perror("Thread Function Argument malloc() failed");
      close(clnt_tcp_sock);
      continue;
    }
    t_arg->sock = clnt_tcp_sock;
    t_arg->playerId = currentPlayerId;

    pthread_t tid;

    int ret = pthread_create(&tid, NULL, recvThreadFunc, (void *)t_arg);
    if (ret != 0) {
      fprintf(stderr, "Client Thread pthread_create() failed: %s\n",
              strerror(ret));
      close(clnt_tcp_sock);
      continue;
    }

    pthread_detach(tid);
    currentPlayerId++;
  }

  free(boardPositions);
  return 0;
}

void *recvThreadFunc(void *arg) {
  RecvThreadArg t_arg = *(RecvThreadArg *)arg;
  int playerId = t_arg.playerId;
  int sock = t_arg.sock;

  GameInitInfo info = gameInitInfo;
  info.playerId = playerId;
  writen(sock, &info, sizeof(info));

  free(arg);
  return NULL;
}

int setOptions(int argc, char *argv[], int *playerCount, int *gridSize,
               int *boardCount, int *gameSec, int *port) {
  int c = 0;
  while ((c = getopt(argc, argv, "n:s:b:t:p:")) != -1) {
    switch (c) {
      case 'n':
        *playerCount = atoi(optarg);
        break;
      case 's':
        *gridSize = atoi(optarg);
        break;
      case 'b':
        *boardCount = atoi(optarg);
        break;
      case 't':
        *gameSec = atoi(optarg);
        break;
      case 'p':
        *port = atoi(optarg);
        break;
      default:
        fprintf(stderr,
                "Usage %s -n <playerNum> -s <gridSize> -b <boardNum> -t "
                "<gameTime> -p <port>\n",
                argv[0]);
        exit(1);
    }
  }
  return 0;
}