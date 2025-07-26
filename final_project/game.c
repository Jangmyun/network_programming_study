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

void transPositionXY(Position *pos, int gridIdx, int gridSize) {
  pos->x = (gridIdx % gridSize) * 2;
  pos->y = (gridIdx / gridSize);
}

int transPositionX(int gridIdx, int gridSize) {
  return (gridIdx % gridSize) * 2;
}
int transPosY(int gridIdx, int gridSize) { return (gridIdx / gridSize) + 1; }

/* =====SERVER===== */

board_pos *generateBoardPosition(GameInitInfo *gameInitInfo) {
  u_int8_t gridSize = gameInitInfo->gridSize;
  u_int16_t boardCount = gameInitInfo->boardCount;
  u_int16_t gridsNum = gridSize * gridSize;

  u_int16_t grids[gridsNum];
  for (int i = 0; i < gridsNum; i++) {
    grids[i] = i;
  }

  // shuffle array
  srand(time(NULL));
  u_int16_t temp, randomIdx;
  for (u_int16_t i = 0; i < gridsNum; i++) {
    randomIdx = rand() % (gridsNum - i) + i;
    temp = grids[i];
    grids[i] = grids[randomIdx];
    grids[randomIdx] = temp;
  }

  // shuffle한 array의 인덱스 0부터 boardCount-1 까지의 내용을 복사
  board_pos *boardPositions = (board_pos *)malloc(sizeof(u_int16_t));
  memcpy(boardPositions, grids, sizeof(board_pos) * boardCount);

  if (boardPositions == NULL) {
    fprintf(stderr, "board position malloc error\n");
    return NULL;
  }

  return boardPositions;
}

void printBoardPositions(board_pos *boardPositions, u_int16_t boardCount) {
  if (boardPositions == NULL) {
    fprintf(stderr, "Board positions is not generated.\n");
    exit(1);
  }

  for (int i = 0; i < boardCount; i++) {
    printf("%d ", boardPositions[i]);
  }
}