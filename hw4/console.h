#ifndef CONSOLE_H
#define CONSOLE_H

#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

extern pthread_mutex_t display_mutex;

void clrscr(void);          // clear screen
void gotoxy(int x, int y);  // move cursor to (x, y) coordinate

int getWindowWidth();   // get width of current console window
int getWindowHeight();  // get width of current console window

void DrawLine(int x1, int y1, int x2, int y2, char c);
void swap(int *pa, int *pb);

void EnableCursor(int enable);
void LockDisplay();
void UnlockDisplay();
void ExclusivePrintF(int indent, const char *format, ...);
void PrintXY(int x, int y, const char *format, ...);
void LogMessageF(const char *filename, int overwrite, const char *format, ...);

void MySleep(int msec);
void MyPause(const char *mesg);

int kbhit();
int getch();

#endif