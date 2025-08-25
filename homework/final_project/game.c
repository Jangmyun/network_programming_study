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
      gotoxy(i * 2 + 1, j + GRID_START_POS);
      printf("  ");
      fflush(stdout);
    }
    gotoxy(gridSize * 2, i + GRID_START_POS);
    printf("%s\033[0K%s", COLOR_RESET, COLOR_BLANK);
  }

  clearColor();

  pthread_mutex_unlock(&display_mutex);
  return;
}

void transPositionXY(Position *pos, int gridIdx, int gridSize) {
  pos->x = (gridIdx % gridSize) * 2 + 1;
  pos->y = (gridIdx / gridSize) + GRID_START_POS;
}

int transPositionX(int gridIdx, int gridSize) {
  return (gridIdx % gridSize) * 2;
}
int transPosY(int gridIdx, int gridSize) { return (gridIdx / gridSize) + 1; }

/* =====SERVER===== */

// 입력받은 각 argument 혹은 기본값이 적절한지 validation check
void validateGameInfo(GameInitInfo *g) {
  int exitCond = 0;

  // player 수는 짝수이며~ 8명 이하
  if (g->playerCount > 8 || g->playerCount < 2 || (g->playerCount % 2) != 0) {
    printError("player count must be even (2,4,6,8)");
    exitCond = 1;
  }
  // size는 짝수이며 최대 32
  if (g->gridSize < 2 || g->gridSize > 32 || g->gridSize % 2 != 0) {
    printError("grid size must be even (2-32)");
    exitCond = 1;
  }
  // boardCount 는 짝수이며 gridSize의 제곱 이하
  if (g->boardCount < 2 || g->boardCount > ((g->gridSize) * (g->gridSize)) ||
      g->boardCount % 2 != 0) {
    printError("board number must be even (2-gridSize^2)");
    exitCond = 1;
  }
  // 최소 10초
  if (g->gameTime.tv_sec < 10) {
    printError("Time should be at lease 10sec");
    exitCond = 1;
  }

  if (exitCond) exit(1);
}

board_pos *generateBoardPosition(GameInitInfo *gameInitInfo,
                                 board_bitarray *ba) {
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
  board_pos *boardPositions =
      (board_pos *)malloc(sizeof(u_int16_t) * boardCount);
  memcpy(boardPositions, grids, sizeof(board_pos) * boardCount);

  // 전체 grid 중 어떤 인덱스가 board인지 표시
  for (int i = 0; i < boardCount; i++) {
    BIT_SET(*ba, boardPositions[i]);
  }

  if (boardPositions == NULL) {
    fprintf(stderr, "board position malloc error\n");
    return NULL;
  }

  return boardPositions;
}

void randomizeBoardColor(board_bitarray *ba, u_int16_t boardCount) {
  u_int16_t boards[boardCount];
  for (int i = 0; i < boardCount; i++) {
    boards[i] = i;
  }

  // shuffle array
  srand(time(NULL));
  u_int16_t temp, randomIdx;
  for (int i = 0; i < boardCount; i++) {
    randomIdx = rand() % (boardCount - i) + i;
    temp = boards[i];
    boards[i] = boards[randomIdx];
    boards[randomIdx] = temp;
  }

  // shuffle한 array의 절반을 blue팀으로 만들기
  for (int i = 0; i < (boardCount / 2); i++) {
    BIT_SET(*ba, boards[i]);
  }
}

void printError(char *errStr) { fprintf(stderr, "%s\n", errStr); }

void printBoardPositions(board_pos *boardPositions, u_int16_t boardCount) {
  if (boardPositions == NULL) {
    fprintf(stderr, "Board positions is not generated.\n");
    exit(1);
  }

  for (int i = 0; i < boardCount; i++) {
    printf("%d ", boardPositions[i]);
  }
}

void printfBoardStatus(board_bitarray *ba, u_int16_t boardCount) {
  int count = 0;
  for (int i = 0; i < boardCount; i++) {
    printf("%d ", BIT_ISSET(*ba, i));
    if (BIT_ISSET(*ba, i)) {
      count++;
    }
  }
  printf("\n%d\n", count);
}

void printGameInfo(GameInitInfo *gii) {
  printf("===GAME INFO===\n");

  printf("playerCount=%d, gridSize=%d, boardCount=%d, gameTime=%ld\n",
         gii->playerCount, gii->gridSize, gii->boardCount,
         gii->gameTime.tv_sec);
}

ssize_t readn(int fd, void *data, size_t n) {
  size_t left;
  size_t read_n;

  left = n;

  while (left > 0) {
    if ((read_n = read(fd, data, left)) < 0) {
      if (left == n)
        return -1;
      else
        break;
    } else if (read_n == 0)
      break;
    left -= read_n;
    data += read_n;
  }

  return n - left;
}

ssize_t writen(int fd, const void *data, size_t n) {
  size_t left;
  ssize_t written_n;

  left = n;

  while (left > 0) {
    if ((written_n = write(fd, data, left)) < 0) {
      if (left == n)
        return -1;
      else
        break;
    } else if (written_n == 0)
      break;
    left -= written_n;
    data += written_n;
  }

  return n - left;
}

int countBlue(board_bitarray *ba, int boardCount) {
  int c = 0;
  for (int i = 0; i < boardCount; i++) {
    if (BIT_ISSET(*ba, i)) {
      c++;
    }
  }

  return c;
}