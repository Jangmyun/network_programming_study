#include "console.h"
#include "search.h"

// User Search Input
#define MAX_INPUT_LEN 1024
int currentInputLen = 0;
char inputStr[MAX_INPUT_LEN];

// Current Cursor Position
int CURSOR_X, CURSOR_Y;

int main() {
  // Get screen size
  int screenWidth = getWindowWidth();
  int screenHeight = getWindowHeight();

  printf("Input > ");

  CURSOR_X = 1;
  CURSOR_Y = screenHeight + 1;

  return 0;
}