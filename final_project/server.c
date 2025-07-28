#include "console.h"
#include "game.h"

#define MULTICAST_GROUP_IP "225.192.0.10"
#define TTL 20

int port;

board_pos *boardPositions = NULL;
u_int16_t playerPositions[8];
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
int setPlayerInitPos(u_int8_t id);
int findBoardIdxByGridIdx(u_int16_t gridIdx);
void *recvThreadFunc(void *arg);  // Client로부터 PlayerAction 받는 스레드
void *sendThreadFunc(void *arg);  // Client에게 GameStatus 보내는 스레드

int main(int argc, char *argv[]) {
  // default options
  port = 5123;
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

  // playerPosition playerCount만큼 동적 할당 후 각 인덱스에 번호 부여
  for (int i = 0; i < gameInitInfo.playerCount; i++) {
    playerPositions[i] = setPlayerInitPos(i);
  }

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

  serv_tcp_sock = socket(PF_INET, SOCK_STREAM, 0);
  serv_udp_sock = socket(PF_INET, SOCK_DGRAM, 0);

  // addr memset
  // TCP
  memset(&serv_tcp_addr, 0, sizeof(serv_tcp_addr));
  serv_tcp_addr.sin_family = AF_INET;
  serv_tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_tcp_addr.sin_port = htons(port);

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

  // Game Status 전송 스레드 생성
  pthread_t sendTid;
  {
    int ret =
        pthread_create(&sendTid, NULL, sendThreadFunc, (void *)&serv_udp_sock);
    if (ret != 0) {
      fprintf(stderr, "Server Thread pthread_create() failed: %s\n",
              strerror(ret));
    }

    pthread_detach(sendTid);
  }

  sleep(gameInitInfo.gameTime.tv_sec);  // game 시간만큼 sleep

  free(boardPositions);
  return 0;
}

void *recvThreadFunc(void *arg) {
  RecvThreadArg t_arg = *(RecvThreadArg *)arg;
  int playerId = t_arg.playerId;
  int sock = t_arg.sock;

  int rw_len;

  // gameInitInfo 전송
  GameInitInfo info = gameInitInfo;
  info.playerId = playerId;
  rw_len = writen(sock, &info, sizeof(info));

  // boardPositions 전송
  for (int i = 0; i < gameInitInfo.boardCount; i++) {
    rw_len = writen(sock, &boardPositions[i], sizeof(board_pos));
  }

  // board_pos_bitarray 전송
  // (grid idx를 표현하는 비트열에서 board position을 모두 1로 set한 비트열)
  rw_len = writen(sock, &board_pos_bitarray, sizeof(board_bitarray));

  // board status 전송
  rw_len = writen(sock, &board_status, sizeof(board_bitarray));

  // playerPositions 전송
  for (int i = 0; i < gameInitInfo.playerCount; i++) {
    writen(sock, &playerPositions[i], sizeof(u_int16_t));
  }

  // PlayerAction을 수신
  PlayerAction pa;
  int boardIdx;
  while (1) {
    readn(sock, &pa, sizeof(PlayerAction));
    LockDisplay();
    playerPositions[playerId] = pa.position;

    if ((boardIdx = findBoardIdxByGridIdx(pa.position)) != -1 &&
        pa.colorFlag != 2) {
      if (pa.colorFlag) {
        BIT_SET(board_status, boardIdx);
      } else {
        BIT_CLR(board_status, boardIdx);
      }
    }
    UnlockDisplay();
#ifdef DEBUG
    printf("PlayerPos=%d , colorFlipped=%d\n", playerPositions[playerId],
           pa.colorFlag);
#endif
  }

  free(arg);
  return NULL;
}

void *sendThreadFunc(void *arg) {
  int sock = *(int *)arg;
  struct sockaddr_in mc_group_addr;

  memset(&mc_group_addr, 0, sizeof(mc_group_addr));
  mc_group_addr.sin_family = AF_INET;
  mc_group_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP_IP);
  mc_group_addr.sin_port = htons(port);  // TCP와 같은 포트

  GameStatus gs;

  while (1) {
    // 현재 game status 복사 후 전송
    memcpy(gs.playerPositions, playerPositions,
           sizeof(u_int16_t) * gameInitInfo.playerCount);
    gs.boardStatus = board_status;

    sendto(sock, &gs, sizeof(GameStatus), 0, (struct sockaddr *)&mc_group_addr,
           sizeof(mc_group_addr));

#ifdef DEBUG
    printf("game status sent\n");
#endif

    MySleep(200);  // 0.1초마다 전송
  }

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

int setPlayerInitPos(u_int8_t id) {
  int playerPositionBranch = id % 4;  // 0,1,2,3

  // 0-좌상단 1-우하단 2-좌하단 3-우상단
  switch (playerPositionBranch) {
    case 0:
      return 0;
    case 1:
      return gameInitInfo.gridSize * gameInitInfo.gridSize - 1;
    case 2:
      return gameInitInfo.gridSize * gameInitInfo.gridSize -
             gameInitInfo.gridSize;
    case 3:
      return gameInitInfo.gridSize - 1;
  }

  return 0;
}
int findBoardIdxByGridIdx(u_int16_t gridIdx) {
  if (!BIT_ISSET(board_pos_bitarray, gridIdx)) return -1;

  for (int i = 0; i < gameInitInfo.boardCount; i++) {
    if (boardPositions[i] == gridIdx) {
      return i;
    }
  }
  return -1;
}