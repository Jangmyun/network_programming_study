#include "console.h"
#include "game.h"

board_pos *boardPositions = NULL;
u_int16_t boardCount = 200;
GameInitInfo gameInitInfo;
board_bitarray board_status;

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
  puts("");
#endif

  // board color randomization
  randomizeBoardColor(&board_status, boardCount);

#ifdef DEBUG
  puts("Random Board Color Selected");
  printfBoardStatus(&board_status, boardCount);
  puts("");
#endif

  free(boardPositions);
  return 0;
}