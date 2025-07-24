#ifndef SEARCH_H
#define SEARCH_H

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define CHAR_SIZE 128

typedef struct Node {
  struct Node* children[CHAR_SIZE];
  uint8_t isEndOfWord;
} Node;

typedef struct {
  char* word;
  Node* trieRoot;
  int wordLength;
  int searchCount;
} Keyword;

Node* createNode();
void insert(Node* root, const char* suffix);
void insertSuffix(Node* root, const char* suffix);
void display(Node* root, char str[], int level);
int search(Node* root, const char* suffix);

int countFileLines(FILE* fp);

Keyword* createKeyword(FILE* fp, int lines);

void printKeywords(Keyword* keywords, int n);

#endif