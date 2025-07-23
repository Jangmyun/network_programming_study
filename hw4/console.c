#include "console.h"

void gotoxy(int x, int y) { printf("%c[%d;%df", 0x1B, y, x); }

void clrscr() {
  fprintf(stdout, "\033[2J\033[0;0f");
  fflush(stdout);
}

int getWindowWidth() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  return w.ws_col;
}

int getWindowHeight() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  return w.ws_row;
}

int getch() {
  int c = 0;
  struct termios oldattr, newattr;

  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  newattr.c_cc[VMIN] = 1;
  newattr.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
  c = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);

  return c;
}

int kbhit(void) {
  struct termios oldt, newt;
  int ch = 0;
  int oldf = 0;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getch();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF) {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

void EnableCursor(int enable) {
  if (enable) {
    printf("\e[?25h");
    fflush(stdout);
  } else {
    printf("\e[?25l");
    fflush(stdout);
  }
}

// mutex for display routines
pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;

void LockDisplay() { pthread_mutex_lock(&display_mutex); }

void UnlockDisplay() { pthread_mutex_unlock(&display_mutex); }

void ExclusivePrintF(int indent, const char *format, ...) {
  va_list vl;

  pthread_mutex_lock(&display_mutex);

  for (int i = 0; i < indent; i++) putchar('\t');

  va_start(vl, format);
  vprintf(format, vl);
  va_end(vl);

  fflush(stdout);

  pthread_mutex_unlock(&display_mutex);
}

void PrintXY(int x, int y, const char *format, ...) {
  va_list vl;

  pthread_mutex_lock(&display_mutex);

  gotoxy(x, y);

  va_start(vl, format);
  vprintf(format, vl);
  va_end(vl);

  fflush(stdout);

  pthread_mutex_unlock(&display_mutex);
}

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void LogMessageF(const char *filename, int overwrite, const char *format, ...) {
  va_list vl;

  pthread_mutex_lock(&log_mutex);

  FILE *fp = fopen(filename, (overwrite ? "w" : "a"));
  if (fp == NULL) {
    printf("Failed to open %s in %s\n", filename, __FUNCTION__);
    fflush(stdout);
    return;
  }

  va_start(vl, format);
  vfprintf(fp, format, vl);
  va_end(vl);

  fclose(fp);

  pthread_mutex_unlock(&log_mutex);
}

void MySleep(int msec) { usleep(msec * 1000); }

void MyPause(const char *mesg) {
  if (mesg) printf("[%s] ", mesg);
  printf("Press Enter to continue...");
  fflush(stdout);
  getchar();
}

void DrawLine(int x1, int y1, int x2, int y2, char c) {
  int dx = 0, dy = 0;
  int x = 0, y = 0;

  if (abs(x2 - x1) > abs(y2 - y1)) {
    if (x1 > x2) {
      swap(&x1, &x2);
      swap(&y1, &y2);
    }
  } else if (y1 > y2) {
    swap(&x1, &x2);
    swap(&y1, &y2);
  }

  dx = x2 - x1;
  dy = y2 - y1;

  if (dx >= dy) {
    if (dx == 0) {
      gotoxy(x1, y1);
      printf("%c", c);
    } else {
      for (x = x1; x <= x2; x++) {
        y = (y1 * (x2 - x) + y2 * (x - x1) + dx / 2) / dx;
        gotoxy(x, y);
        printf("%c", c);
      }
    }
  } else {
    for (y = y1; y <= y2; y++) {
      x = (x1 * (y2 - y) + x2 * (y - y1) + dy / 2) / dy;
      gotoxy(x, y);
      printf("%c", c);
    }
  }
}

void swap(int *pa, int *pb) {
  int temp = *pa;
  *pa = *pb;
  *pb = temp;
}
