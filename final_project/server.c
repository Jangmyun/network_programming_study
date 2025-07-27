#include "console.h"
#include "game.h"

board_pos *boardPositions = NULL;
GameInitInfo gameInitInfo;
board_bitarray board_pos_bitarray;  // 어떤 위치가 board인지
board_bitarray board_status;        // board들의 상태 (blue, red);

int currPlayer = 0;

int setOptions(int argc, char *argv[], int *playerCount, int *gridSize,
               int *boardCount, int *gameSec, int *port);

int main(int argc, char *argv[]) {
  // default options
  int port = 5123;
  {
    int playerCount = 2;
    int gridSize = 32;
    int boardCount = 200;
    int gameSec = 60;

    setOptions(argc, argv, &playerCount, &gridSize, &boardCount, &gameSec,
               &port);

    struct timeval gameTime = {gameSec, 0};
    setGameInitInfo(&gameInitInfo, playerCount, currPlayer, gridSize,
                    boardCount, boardPositions, gameTime);

    if (port < 1024 || port > 65535) {
      printError("Cannot use well-known port");
      exit(1);
    }

    validateGameInfo(&gameInitInfo);
  }

#ifdef DEBUG
  puts("GameInitInfo Set");
  printGameInfo(&gameInitInfo);
  printf("PORT = %d\n", port);
#endif

  // 설정한 보드의 개수만큼 전체 grid 인덱스에서 보드의 위치를 랜덤하게 설정
  boardPositions = generateBoardPosition(&gameInitInfo, &board_pos_bitarray);

#ifdef DEBUG
  puts("Random Board Positions Generated");
  printBoardPositions(boardPositions, gameInitInfo.boardCount);
  puts("");
#endif

  // 선택한 보드들 중 랜덤한 절반의 flag를 1로 설정
  randomizeBoardColor(&board_status, gameInitInfo.boardCount);

#ifdef DEBUG
  puts("Random Board Color Selected");
  printfBoardStatus(&board_status, gameInitInfo.boardCount);
  puts("");
#endif

  free(boardPositions);
  return 0;
}

int setOptions(int argc, char *argv[], int *playerCount, int *gridSize,
               int *boardCount, int *gameSec, int *port) {
  int c = 0;
  while ((c = getopt(argc, argv, "n:s:b:t:p:")) != -1) {
    switch (c) {
      case 'n':
        *playerCount = atoi(optarg);
        break;
      case 's':
        *gridSize = atoi(optarg);
        break;
      case 'b':
        *boardCount = atoi(optarg);
        break;
      case 't':
        *gameSec = atoi(optarg);
        break;
      case 'p':
        *port = atoi(optarg);
        break;
      default:
        fprintf(stderr,
                "Usage %s -n <playerNum> -s <gridSize> -b <boardNum> -t "
                "<gameTime> -p <port>\n",
                argv[0]);
        exit(1);
    }
  }
  return 0;
}