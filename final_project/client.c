#include <unistd.h>

#include "console.h"
#include "game.h"

u_int16_t *boardPositions = NULL;
u_int16_t boardCount = 200;
GameInitInfo gameInitInfo;

int window_x;
int window_y;

void drawBoards();

int main(int arc, char *argv[]) {
  struct timeval gameTime = {60, 0};
  setGameInitInfo(&gameInitInfo, 2, 1, 32, 200, boardPositions, gameTime);
  boardPositions = generateBoardPosition(&gameInitInfo);

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

void drawBoards() {
  Position pos;
  LockDisplay();
  for (int i = 0; i < boardCount; i++) {
    transPositionXY(&pos, boardPositions[i], gameInitInfo.gridSize);
    setColor(COLOR_RED_BOARD);
    gotoxy(pos.x, pos.y + GRID_START_POS);
    printf("  ");
    clearColor();
    fflush(stdout);
  }
  UnlockDisplay();
}