#ifndef GAME_H
#define GAME_H

#define MAX_GRID_SIZE 32
#define MAX_BOARD (MAX_GRID_SIZE * MAX_GRID_SIZE)
typedef long board_mask;
#define BOARD_BITS (8 * sizeof(board_mask))

#include <sys/types.h>

// 보드 위치 표현을 위한 1024 비트열
typedef struct {
  board_mask board_bits[MAX_BOARD / BOARD_BITS];
} board_bitarray;

// 서버가 클라이언트 각각에 보내는 Game 준비용
typedef struct _GameInitInfo {
  u_int8_t playerCount;  // 총 플레이어 수
  u_int8_t playerId;     // 서버가 클라이언트에게 부여한 고유 번호 (접속 순서)
  u_int8_t gridSize;     // 한 변의 크기
  u_int16_t boardCount;  // 전체 판 개수
  u_int16_t boardPosition[];
} GameInitInfo;

#endif