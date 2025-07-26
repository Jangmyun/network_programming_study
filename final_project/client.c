#include <unistd.h>

#include "console.h"
#include "game.h"

u_int16_t *boardPositions = NULL;
u_int16_t boardCount = 200;
GameInitInfo gameInitInfo;
board_bitarray board_status;

int window_x;
int window_y;

u_int16_t playerPos;
int setPlayerInitPos(u_int8_t id);
void drawBoards();

int main(int arc, char *argv[]) {
  struct timeval gameTime = {60, 0};
  setGameInitInfo(&gameInitInfo, 2, 1, 32, 200, boardPositions, gameTime);
  boardPositions = generateBoardPosition(&gameInitInfo);
  randomizeBoardColor(&board_status, boardCount);

  window_x = getWindowWidth();
  window_y = getWindowHeight();

  clrscr();

  drawGrid(gameInitInfo.gridSize);

#ifdef DEBUG
  printBoardPositions(boardPositions, boardCount);
#endif

  drawBoards();

  sleep(5);

  gotoxy(window_x, window_y);
  free(boardPositions);
  return 0;
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
  for (int i = 0; i < boardCount; i++) {
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