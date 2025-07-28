#include <unistd.h>

#include "console.h"
#include "game.h"

#define MULTICAST_GROUP_IP "225.192.0.10"

board_pos *boardPositions = NULL;
u_int16_t *playerPositions = NULL;
GameInitInfo gameInitInfo;
board_bitarray board_pos_bitarray;
board_bitarray board_status;

int window_x;
int window_y;

u_int16_t playerPos;

void drawBoards();
void drawPlayer();
void *drawGame(void *arg);

int findBoardIdxByGridIdx(u_int16_t gridIdx);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <IP> <port>\n", argv[0]);
    exit(1);
  }

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
  playerPositions = (u_int16_t *)malloc(
      sizeof(u_int16_t) * gameInitInfo.playerCount);  // player 수 만큼 할당

  for (int i = 0; i < gameInitInfo.playerCount; i++) {
    rw_len = readn(tcp_sock, &playerPositions[i], sizeof(u_int16_t));
  }

  gameInitInfo.boardPositions = boardPositions;

  // 받아온 초기 플레이어 위치로 playerPos 초기화
  playerPos = playerPositions[gameInitInfo.playerId];

  window_x = getWindowWidth();
  window_y = getWindowHeight();

  EnableCursor(0);  // disable cursor

#ifdef DEBUG
  printBoardPositions(boardPositions, gameInitInfo.boardCount);
#endif

  clrscr();

  pthread_t drawerTid;
  pthread_create(&drawerTid, NULL, drawGame, NULL);

  pthread_detach(drawerTid);

  while (1) {
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
        case ENTER:
          PrintXY(1, window_y, "PlayerPos = %d", newPlayerPos);
          break;
      }

      Position new, prev;
      transPositionXY(&new, newPlayerPos, gameInitInfo.gridSize);
      transPositionXY(&prev, playerPos, gameInitInfo.gridSize);

      if (newPlayerPos != playerPos) {
        int ret = findBoardIdxByGridIdx(playerPos);
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
        drawPlayer();
      }
    }
  }

  EnableCursor(1);
  gotoxy(window_x, window_y);
  free(playerPositions);
  free(boardPositions);
  return 0;
}

void *drawGame(void *arg) {
  int myId = gameInitInfo.playerId;

  while (1) {
    drawGrid(gameInitInfo.gridSize);
    drawBoards();
    drawPlayer();
    MySleep(100);
  }

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

void drawPlayer() {
  Position p;
  transPositionXY(&p, playerPos, gameInitInfo.gridSize);

  LockDisplay();

  int ret = findBoardIdxByGridIdx(playerPos);
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
  printf("[]");
  fflush(stdout);
  clearColor();
  UnlockDisplay();
}

// blank 위치면 -1, board 위치면 red, blue 각각 0, 1 리턴
int findBoardIdxByGridIdx(u_int16_t gridIdx) {
  if (BIT_ISSET(board_pos_bitarray, gridIdx)) {
    for (int i = 0; i < gameInitInfo.boardCount; i++) {
      if (boardPositions[i] == gridIdx) {
        if (BIT_ISSET(board_status, i)) {
          return 1;
        } else {
          return 0;
        }
      }
    }
  } else {
    return -1;
  }
  return -1;
}