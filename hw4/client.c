#include <ctype.h>

#include "console.h"
#include "search.h"

// User Search Input
#define MAX_INPUT_LEN 1024
int currentInputLen = 0;
char inputStr[MAX_INPUT_LEN];

#define ESC 27
#define BS 8
#define DEL 127

#define INPUT_START_POS 14

// Current Cursor Position
int CURSOR_X, CURSOR_Y;

void refreshInput();
void showSearchResult();

void *receiveThreadFunc(void *arg);
void sendWord(int sock);
void recvMatchedWords(int sock, int matchedCount, Keyword matchedKeywords[]);
void printMatchedWords(int matchedCount, Keyword keywords[]);
void sortMatchedWords(int matchedCount, Keyword Keywords[]);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int sock = 0;
  int rw_len;
  struct sockaddr_in serv_addr;

  sock = socket(PF_INET, SOCK_STREAM, 0);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  // Get screen size
  int screenWidth = getWindowWidth();
  int screenHeight = getWindowHeight();

  CURSOR_X = INPUT_START_POS;
  CURSOR_Y = 1;

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    errorExit("connect() error");
  }

  clrscr();

  printf("\033[01;33mSearch Word:\033[0m ");

  gotoxy(CURSOR_X, CURSOR_Y);

  pthread_t tid;
  int ret = pthread_create(&tid, NULL, receiveThreadFunc, (void *)&sock);
  if (ret != 0) {
    fprintf(stderr, "pthread_create() failed %s\n", strerror(ret));
    close(sock);
  }

  while (1) {
    // 키보드 입력 처리
    if (kbhit()) {
      int key = getch();

      if (key == ESC) {
        break;
      }

      // 입력 문자가 알파벳이나 스페이스일 경우
      if (isalpha(key) || key == ' ') {
        if (currentInputLen < MAX_INPUT_LEN - 1) {
          inputStr[currentInputLen++] = key;
          inputStr[currentInputLen] = '\0';
          refreshInput();
        }
      }
      // backspace(^H) 인 경우
      if (key == BS) {
        if (currentInputLen > 0) {
          inputStr[--currentInputLen] = '\0';
          refreshInput();
        }
      }

      // 입력이 있는 상태라면 word를 서버에 전송
      if (currentInputLen > 0) {
        PrintXY(1, 2, "%s sent", inputStr);
        sendWord(sock);
      }
    }
    usleep(10000);
  }
  PrintXY(1, 2, "Bye!\n");

  return 0;
}

void sendWord(int sock) {
  size_t wordLen = currentInputLen;
  writen(sock, &wordLen, sizeof(size_t));
  writen(sock, inputStr, currentInputLen);
}

void *receiveThreadFunc(void *arg) {
  int sock = *(int *)arg;

  char buf[BUF_SIZE];

  while (1) {
    int matchedCount = 0;
    readn(sock, &matchedCount, sizeof(int));

#ifdef DEBUG
    PrintXY(1, 3, "%d", matchedCount);
#endif

    Keyword matchedKeywords[matchedCount];

    recvMatchedWords(sock, matchedCount, matchedKeywords);

    printMatchedWords(matchedCount, matchedKeywords);

    LockDisplay();
    gotoxy(INPUT_START_POS + currentInputLen, 1);
    UnlockDisplay();
  }

  return NULL;
}

void recvMatchedWords(int sock, int matchedCount, Keyword matchedKeywords[]) {
  int rw_len;

  // 수신한 keywords 개수만큼 word의 길이와 word와 searchCount 수신
  int wordLen = 0;
  int searchCount = 0;
  for (int i = 0; i < matchedCount; i++) {
    // word 길이 수신
    rw_len = readn(sock, &wordLen, sizeof(int));
    matchedKeywords[i].wordLength = wordLen;
    // word 길이만큼 word 수신
    char *word = (char *)malloc(sizeof(char) * wordLen);
    rw_len = readn(sock, word, wordLen);
    matchedKeywords[i].word = word;
    // searchCount 수신
    rw_len = readn(sock, &searchCount, sizeof(int));
    matchedKeywords[i].searchCount = searchCount;

#ifdef DEBUG
    printf("[%d] %s, %d", matchedKeywords[i].searchCount,
           matchedKeywords[i].word, matchedKeywords[i].wordLength);
#endif
  }
}

void refreshInput() {
  pthread_mutex_lock(&display_mutex);
  gotoxy(INPUT_START_POS, 1);

  printf("\033[K");
  printf("%s", inputStr);

  fflush(stdout);
  pthread_mutex_unlock(&display_mutex);
}

void printMatchedWords(int matchedCount, Keyword keywords[]) {
  const int Y_OFFSET = 3;

  pthread_mutex_lock(&display_mutex);
  gotoxy(1, Y_OFFSET);
  printf("\033[0J");
  pthread_mutex_unlock(&display_mutex);

  for (int i = 0; i < matchedCount; i++) {
    pthread_mutex_lock(&display_mutex);

    gotoxy(1, Y_OFFSET + i);
    printf("\033[K");

    pthread_mutex_unlock(&display_mutex);

    PrintXY(1, Y_OFFSET + i, "%s", keywords[i].word);
  }
}