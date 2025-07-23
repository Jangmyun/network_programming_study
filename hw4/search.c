#include "search.h"

Node* createNode() {
  Node* node = (Node*)malloc(sizeof(Node));
  node->isEndOfWord = 0;
  for (int i = 0; i < CHAR_SIZE; i++) {
    node->children[i] = NULL;
  }
  return node;
}

void insert(Node* root, const char* suffix) {
  Node* current = root;
  for (int i = 0; suffix[i] != '\0'; i++) {
    int idx = suffix[i];
    if (current->children[idx] == NULL) {
      current->children[idx] = createNode();
    }
  }
  current->isEndOfWord = 1;
}

void display(Node* root, char str[], int level) {
  if (root->isEndOfWord) {
    str[level] = '\0';
    printf("%s\n", str);
  }
  for (int i = 0; i < CHAR_SIZE; i++) {
    if (root->children[i] != NULL) {
      str[level] = i + 'a';
      display(root->children[i], str, level + 1);
    }
  }
}

int search(Node* root, const char* suffix) {
  Node* current = root;
  for (int i = 0; suffix[i] != '\0'; i++) {
    int idx = suffix[i];
    if (current->children[idx] == NULL) {
      return 0;
    }
    current = current->children[idx];
  }
  return current->isEndOfWord;
}