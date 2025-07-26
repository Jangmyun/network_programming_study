#include "console.h"
#include "game.h"

board_pos *boardPositions = NULL;
u_int16_t boardCount = 200;
GameInitInfo gameInitInfo;

int main(int arc, char *argv[]) {
  struct timeval gameTime = {60, 0};
  setGameInitInfo(&gameInitInfo, 2, 1, 32, 200, boardPositions, gameTime);

#ifdef DEBUG
  puts("GameInitInfo Set");
#endif

  // random board position generation
  boardPositions = generateBoardPosition(&gameInitInfo);

#ifdef DEBUG
  puts("Random Board Positions Generated");
  printBoardPositions(boardPositions, gameInitInfo.boardCount);
#endif

  free(boardPositions);
  return 0;
}