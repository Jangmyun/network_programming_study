#include "game.h"#include "game.h"
#include "game.h"

#include "console.h"

/* =====COMMON===== */
void setGameInitInfo(GameInitInfo *gameInfo, u_int8_t playerCount,
                     u_int8_t playerId, u_int8_t gridSize, u_int16_t boardCount,
                     board_pos *boardPosition, struct timeval gameTime) {
  gameInfo->playerCount = playerCount;
  gameInfo->playerId = playerId;
  gameInfo->gridSize = gridSize;
  gameInfo->boardCount = boardCount;
  gameInfo->boardPositions = boardPosition;
  gameInfo->gameTime = gameTime;
  return;
}

/* =====CLIENT===== */

void setColor(char *color) { printf("%s", color); }
void clearColor() { printf("\033[39;49m"); }

void drawGrid(int gridSize) {
  pthread_mutex_lock(&display_mutex);

  setColor(COLOR_BLANK);

  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      gotoxy(i * 2, j + GRID_START_POS);
      printf("  ");
    }
  }

  clearColor();

  fflush(stdout);
  pthread_mutex_unlock(&display_mutex);
  return;
}

