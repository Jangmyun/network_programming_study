#ifndef SEARCH_H
#define SEARCH_H

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define CHAR_SIZE 256

typedef struct Node {
  struct Node* children[CHAR_SIZE];
  uint8_t isEndOfWord;
} Node;

typedef struct {
  char* searchWord;
  Node* trieRoot;
  int word_length;
} Keyword;

Node* createNode();
void insert(Node* root, const char* suffix);
void display(Node* root, char str[], int level);
int search(Node* root, const char* suffix);

#endif