#ifndef GAME_H
#define GAME_H

#define MAX_GRID_SIZE 32
#define MAX_BOARD (MAX_GRID_SIZE * MAX_GRID_SIZE)
typedef long board_mask;
#define BOARD_BITS (8 * sizeof(board_mask))

typedef unsigned short u_int16_t;
typedef u_int16_t board_pos;

/* =====CLIENT===== */
#define GRID_START_POS 7
#define BLANK_PER_GRID 2  // 하나의 GRID를 공백 두개로 표현

#define COLOR_RESET "\033[0m"
#define COLOR_BLANK "\033[40m"
#define COLOR_PLAYER "\033[47m"
#define COLOR_RED_TEAM "\033[101m"
#define COLOR_RED_BOARD "\033[41m"
#define COLOR_BLUE_TEAM "\033[104m"
#define COLOR_BLUE_BOARD "\033[44m"

// Player가 서 있는 위치에 따른 색 변화
#define COLOR_PLAYER_ON_BLUE "\033[104m"
#define COLOR_PLAYER_ON_RED "\033[101m"

// 팀 문자열
#define RED_CHAR "<>"
#define BLUE_CHAR "()"

#define ENTER 13

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

// 1024 bit array structure for board positions
typedef struct {
  board_mask board_bits[MAX_BOARD / BOARD_BITS];
} board_bitarray;

// 비트 연산 매크로
#define BIT_SET(bitarray, pos) \
  ((bitarray).board_bits[(pos) / BOARD_BITS] |= (1L << (pos % BOARD_BITS)))
#define BIT_CLR(bitarray, pos) \
  ((bitarray).board_bits[(pos) / BOARD_BITS] &= ~(1L << (pos % BOARD_BITS)))
#define BIT_ISSET(bitarray, pos)                                               \
  (((bitarray).board_bits[(pos) / BOARD_BITS] & (1L << (pos % BOARD_BITS))) != \
   0)

typedef struct {
  u_int8_t x;
  u_int8_t y;
} Position;

// 서버가 클라이언트 각각에 보내는 Game 준비용
typedef struct _GameInitInfo {
  u_int8_t playerCount;  // 총 플레이어 수
  u_int8_t playerId;     // 서버가 클라이언트에게 부여한 고유 번호 (접속 순서)
  u_int8_t gridSize;     // 한 변의 크기
  u_int16_t boardCount;  // 전체 판 개수
  board_pos *boardPositions;
  struct timeval gameTime;
} GameInitInfo;

typedef struct _PlayerAction {
  u_int16_t position;
  int colorFlag;
} PlayerAction;

typedef struct _GameStatus {
  u_int16_t playerPositions[8];
  board_bitarray boardStatus;
} GameStatus;

/* =====COMMON===== */

// GameIninInfo setter
void setGameInitInfo(GameInitInfo *gameInfo, u_int8_t playerCount,
                     u_int8_t playerId, u_int8_t gridSize, u_int16_t boardCount,
                     board_pos *boardPosition, struct timeval gameTime);

/* =====CLIENT===== */

void setColor(char *color);
void clearColor();

void transPositionXY(Position *pos, int gridIdx, int gridSize);
int transPositionX(int gridIdx, int gridSize);
int transPosY(int gridIdx, int gridSize);

void drawGrid(int gridSize);

/* =====SERVER===== */

void validateGameInfo(GameInitInfo *gii);

// Grid안에서 판의 위치 랜덤 생성
board_pos *generateBoardPosition(GameInitInfo *gameInitInfo,
                                 board_bitarray *ba);
void randomizeBoardColor(board_bitarray *ba, u_int16_t boardCount);

// for debugging
void printError(char *errStr);
void printBoardPositions(board_pos *boardPositions, u_int16_t boardCount);
void printfBoardStatus(board_bitarray *ba, u_int16_t boardCount);
void printGameInfo(GameInitInfo *gii);

ssize_t readn(int fd, void *data, size_t n);
ssize_t writen(int fd, const void *data, size_t n);
#endif
