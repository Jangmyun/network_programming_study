#include <unistd.h>

#include "console.h"
#include "game.h"

u_int16_t *boardPositions = NULL;
u_int16_t *otherPlayerPos = NULL;
GameInitInfo gameInitInfo;
board_bitarray board_pos_bitarray;
board_bitarray board_status;

int window_x;
int window_y;

u_int16_t playerPos;

int setPlayerInitPos(u_int8_t id);
void drawBoards();
void drawPlayer();
void *drawGame(void *arg);

int findBoardIdxByGridIdx(u_int16_t gridIdx);

int main(int arc, char *argv[]) {
  struct timeval gameTime = {60, 0};
  setGameInitInfo(&gameInitInfo, 2, 1, 32, 200, boardPositions, gameTime);
  boardPositions = generateBoardPosition(&gameInitInfo, &board_pos_bitarray);
  randomizeBoardColor(&board_status, gameInitInfo.boardCount);

  playerPos = setPlayerInitPos(gameInitInfo.playerId);

  window_x = getWindowWidth();
  window_y = getWindowHeight();

  EnableCursor(0);

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

  gotoxy(window_x, window_y);
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

int setPlayerInitPos(u_int8_t id) {
  // 최대 플레이어 수 8명 (id 0-7) 이므로 4로 나눈 나머지를 기준으로 위치 지정
  int playerPosBranch = id / 4;

  switch (playerPosBranch) {
    case 0:
      return 0;
    case 1:
      return gameInitInfo.gridSize * gameInitInfo.gridSize;
    case 2:
      return 0;
    case 3:
      return gameInitInfo.gridSize * gameInitInfo.gridSize;
    default:
      return 0;
  }

  return 0;
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
  clearColor();
  fflush(stdout);
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