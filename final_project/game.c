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

