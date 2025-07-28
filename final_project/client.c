#include <signal.h>
#include <unistd.h>

#include "console.h"
#include "game.h"

#define MULTICAST_GROUP_IP "225.192.0.10"

typedef struct _RecvThreadArg {
  int sock;
  char ipAddr[20];
} RecvThreadArg;

board_pos *boardPositions = NULL;
u_int16_t playerPositions[8];
GameInitInfo gameInitInfo;
board_bitarray board_pos_bitarray;
board_bitarray board_status;

GameTime gameTime;
int gameEndFlag = 0;

int window_x;
int window_y;

u_int16_t playerPos;

void drawBoards();
void drawPlayer(u_int16_t pos, char *icon);
void drawOtherPlayers();
void drawTime();

// Thread functions
void *drawGame(void *arg);
void *recvGameStatus(void *arg);

void drawGameResult(int sock);

void timeout(int signum);

int findBoardIdxByGridIdx(u_int16_t gridIdx);
int findBoardColorByGridIdx(u_int16_t gridIdx);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <IP> <port>\n", argv[0]);
    exit(1);
  }

  struct termios termState;
  tcgetattr(STDIN_FILENO, &termState);

  int tcp_sock;
  struct sockaddr_in serv_tcp_addr;

  int udp_sock;
  struct sockaddr_in mc_group_addr;
  socklen_t mc_group_addr_len;
  struct ip_mreq mreq;

  int rw_len;

  tcp_sock = socket(PF_INET, SOCK_STREAM, 0);
  udp_sock = socket(PF_INET, SOCK_DGRAM, 0);

  // addr memset
  // TCP
  memset(&serv_tcp_addr, 0, sizeof(serv_tcp_addr));
  serv_tcp_addr.sin_family = AF_INET;
  serv_tcp_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_tcp_addr.sin_port = htons(atoi(argv[2]));
  // UDP
  memset(&mc_group_addr, 0, sizeof(mc_group_addr));
  mc_group_addr.sin_family = AF_INET;
  mc_group_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  mc_group_addr.sin_port = htons(atoi(argv[2]));

  if (connect(tcp_sock, (struct sockaddr *)&serv_tcp_addr,
              sizeof(serv_tcp_addr)) == -1) {
    perror("tcp socket connect() failed");
    close(tcp_sock);
    exit(1);
  }

  int optval = 1;
  if (setsockopt(udp_sock, SOL_SOCKET, SO_REUSEPORT, (char *)&optval,
                 sizeof(optval))) {
    perror("udp setsockopt() (SO_REUSEPORT) error");
    close(udp_sock);
    exit(1);
  }

  if (setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                 sizeof(optval))) {
    perror("udp setsockopt() (SO_REUSEADDR) error");
    close(udp_sock);
    exit(1);
  }

  mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP_IP);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(udp_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mreq,
                 sizeof(mreq))) {
    perror("udp setsockopt() (IP_ADD_MEMBERSHIP) error");
    close(udp_sock);
    exit(1);
  }

  // multicast receiver socket binding
  if (bind(udp_sock, (struct sockaddr *)&mc_group_addr,
           sizeof(mc_group_addr)) == -1) {
    perror("udp socket bind() error");
    close(udp_sock);
    exit(1);
  }

  // GameInitInfo 받아오기
  // gameInitInfo update
  rw_len = readn(tcp_sock, &gameInitInfo, sizeof(GameInitInfo));

  // 받아온 gameInitInfo 기반으로 board count만큼 boardPosition 동적할당
  boardPositions =
      (board_pos *)malloc(sizeof(board_pos) * gameInitInfo.boardCount);

  // 반복하여 boardPosition 수신
  for (int i = 0; i < gameInitInfo.boardCount; i++) {
    rw_len = readn(tcp_sock, &boardPositions[i], sizeof(board_pos));
  }
  gameInitInfo.boardPositions = boardPositions;

  // board_position을 모두 1로 설정한 bitarray 수신
  rw_len = readn(tcp_sock, &board_pos_bitarray, sizeof(board_bitarray));

  rw_len = readn(tcp_sock, &board_status, sizeof(board_bitarray));

  // otherPlayerPosition 정보 가져오기
  for (int i = 0; i < gameInitInfo.playerCount; i++) {
    rw_len = readn(tcp_sock, &playerPositions[i], sizeof(u_int16_t));
  }

  // 받아온 초기 플레이어 위치로 playerPos 초기화
  playerPos = playerPositions[gameInitInfo.playerId];

  window_x = getWindowWidth();
  window_y = getWindowHeight();

  int playerPosColor;

  EnableCursor(0);  // disable cursor

#ifdef DEBUG
  printBoardPositions(boardPositions, gameInitInfo.boardCount);
#endif

  clrscr();

  LockDisplay();
  gotoxy(1, 1);
  printf(
      "Board Flipper: \033[93mYour Team:%s\033[0m    (YOU=\"[]\", "
      "\033[34mBLUE=\"()\"\033[0m, "
      "\033[34mRED=\"<>\\033[0m)",
      gameInitInfo.playerId % 2 == 0 ? "RED" : "BLUE");
  UnlockDisplay();

  // Game 시간 정보 수신
  rw_len = readn(tcp_sock, &gameTime, sizeof(GameTime));

  // timer 설정
  signal(SIGALRM, timeout);

  pthread_t drawerTid;
  pthread_create(&drawerTid, NULL, drawGame, NULL);

  pthread_detach(drawerTid);

  RecvThreadArg *arg = (RecvThreadArg *)malloc(sizeof(RecvThreadArg));
  arg->sock = udp_sock;
  strncpy(arg->ipAddr, argv[1], sizeof(arg->ipAddr) - 1);

  pthread_t recvTid;
  pthread_create(&recvTid, NULL, recvGameStatus, (void *)arg);

  pthread_detach(recvTid);

  PlayerAction pa;

  // game 전까지 게임 시작 대기
  while (gameTime.startTime > time(NULL)) {
    PrintXY(1, 3, "Game Ready... %d", gameTime.startTime - time(NULL));
    MySleep(50);
  }

  PrintXY(1, 3, "Game Start !!!!!!!!!!");

  alarm(gameInitInfo.gameTime.tv_sec);

  while (!gameEndFlag) {
    if (kbhit()) {
      int key = getch();

      int newPlayerPos = playerPos;
      switch (key) {
        case 'w':
          if (playerPos >= gameInitInfo.gridSize) {
            newPlayerPos -= gameInitInfo.gridSize;
          }
          break;
        case 'a':
          if (playerPos % gameInitInfo.gridSize != 0) {
            newPlayerPos--;
          }
          break;
        case 's':
          if (playerPos < gameInitInfo.gridSize * (gameInitInfo.gridSize - 1)) {
            newPlayerPos += gameInitInfo.gridSize;
          }
          break;
        case 'd':
          if ((playerPos + 1) % gameInitInfo.gridSize != 0) {
            newPlayerPos++;
          }
          break;
        case ' ':
          pa.position = playerPos;
          playerPosColor = findBoardColorByGridIdx(playerPos);
          if (playerPosColor != -1) {
            int boardIdx = findBoardIdxByGridIdx(playerPos);
            pa.colorFlag = !playerPosColor;
            LockDisplay();
            if (pa.colorFlag) {
              BIT_SET(board_status, boardIdx);
            } else {
              BIT_CLR(board_status, boardIdx);
            }
            UnlockDisplay();
            drawBoards();
            // 뒤집기
          }
          writen(tcp_sock, &pa, sizeof(pa));
          break;
      }

      Position new, prev;
      transPositionXY(&new, newPlayerPos, gameInitInfo.gridSize);
      transPositionXY(&prev, playerPos, gameInitInfo.gridSize);

      if (newPlayerPos != playerPos) {
        int ret = findBoardColorByGridIdx(playerPos);

        pa.position = newPlayerPos;
        pa.colorFlag = 2;

        writen(tcp_sock, &pa, sizeof(pa));

        LockDisplay();
        switch (ret) {
          case -1:  // BLANK
            setColor(COLOR_BLANK);
            break;
          case 0:  // RED
            setColor(COLOR_RED_BOARD);
            break;
          case 1:  // BLUE
            setColor(COLOR_BLUE_BOARD);
            break;
          default:
            setColor(COLOR_BLANK);
            break;
        }

        gotoxy(prev.x, prev.y);
        printf("  ");
        clearColor();
        UnlockDisplay();

        playerPos = newPlayerPos;
        drawPlayer(playerPos, PLAYER_CHAR);
      }
    }
  }

  drawGameResult(tcp_sock);

  tcsetattr(STDIN_FILENO, TCSANOW, &termState);
  free(boardPositions);
  return 0;
}

void *drawGame(void *arg) {
  int myId = gameInitInfo.playerId;

  while (!gameEndFlag) {
    drawGrid(gameInitInfo.gridSize);
    drawBoards();
    drawOtherPlayers();
    drawPlayer(playerPos, PLAYER_CHAR);
    drawTime();
    MySleep(100);
  }

  return NULL;
}

void drawTime() {
  int leftTime = gameTime.endTime - time(NULL);
  LockDisplay();
  gotoxy(1, 5);
  printf("\033[0K");
  printf("TIME LEFT: \033[92m%ld\033[0m",
         leftTime > gameInitInfo.gameTime.tv_sec ? gameInitInfo.gameTime.tv_sec
                                                 : leftTime);

  UnlockDisplay();
}

void *recvGameStatus(void *arg) {
  RecvThreadArg *t_arg = (RecvThreadArg *)arg;

  GameStatus recv_gs;

  struct sockaddr_in mc_group_addr;
  socklen_t mc_group_addr_size = sizeof(struct sockaddr_in);

  int r_len;

  while (!gameEndFlag) {
    r_len = recvfrom(t_arg->sock, &recv_gs, sizeof(GameStatus), 0,
                     (struct sockaddr *)&mc_group_addr, &mc_group_addr_size);

    char *recv_ipStr = inet_ntoa(mc_group_addr.sin_addr);

    // TODO: 멀티 캐스팅 ip
    // if (!strcmp(recv_ipStr, t_arg->ipAddr)) {
    PrintXY(1, 56, "recvfrom");

    memcpy(playerPositions, recv_gs.playerPositions,
           sizeof(u_int16_t) * gameInitInfo.playerCount);
    board_status = recv_gs.boardStatus;
    // }
  }

  free(arg);
  return NULL;
}

void drawBoards() {
  Position pos;
  LockDisplay();
  for (int i = 0; i < gameInitInfo.boardCount; i++) {
    transPositionXY(&pos, boardPositions[i], gameInitInfo.gridSize);

    if (BIT_ISSET(board_status, i)) {
      setColor(COLOR_BLUE_BOARD);
    } else {
      setColor(COLOR_RED_BOARD);
    }

    gotoxy(pos.x, pos.y);
    printf("  ");
    clearColor();
    fflush(stdout);
  }
  UnlockDisplay();
}

void drawPlayer(u_int16_t pos, char *icon) {
  Position p;
  transPositionXY(&p, pos, gameInitInfo.gridSize);

  LockDisplay();

  int ret = findBoardColorByGridIdx(pos);
  switch (ret) {
    case -1:  // BLANK
      setColor(COLOR_BLANK);
      break;
    case 0:  // RED
      setColor(COLOR_PLAYER_ON_RED);
      break;
    case 1:  // BLUE
      setColor(COLOR_PLAYER_ON_BLUE);
      break;
    default:
      setColor(COLOR_BLANK);
      break;
  }
  gotoxy(p.x, p.y);
  printf("%s", icon);
  fflush(stdout);
  clearColor();
  UnlockDisplay();
}

void drawOtherPlayers() {
  for (int i = 0; i < gameInitInfo.playerCount; i++) {
    if (i != gameInitInfo.playerId) {
      drawPlayer(playerPositions[i], (i % 2 == 0 ? RED_CHAR : BLUE_CHAR));
    }
  }
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

// blank 위치면 -1, board 위치면 red, blue 각각 0, 1 리턴
int findBoardColorByGridIdx(u_int16_t gridIdx) {
  int boardIdx;
  if ((boardIdx = findBoardIdxByGridIdx(gridIdx)) != -1) {
    if (BIT_ISSET(board_status, boardIdx)) {
      return 1;
    } else {
      return 0;
    }
  }

  return -1;
}

void timeout(int sig) {
  if (sig == SIGALRM) {
    PrintXY(1, 5, "TIME OUT !!!!!!!!!!");
    gameEndFlag = 1;
  }
}

void drawGameResult(int sock) {
  int blueCount;
  readn(sock, &blueCount, sizeof(int));

  int redCount = gameInitInfo.boardCount - blueCount;

  char *resultStr = blueCount == redCount  ? "DRAW!!"
                    : blueCount > redCount ? "BLUE WIN!!!"
                                           : "RED WIN!!!";

  PrintXY(1, 6, "Game Result : %s", resultStr);
  PrintXY(1, 7, "BLUE:%d | RED:%d", blueCount, redCount);
}