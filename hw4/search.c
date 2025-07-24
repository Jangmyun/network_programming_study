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
    current = current->children[idx];
  }
  current->isEndOfWord = 1;
}

void insertSuffix(Node* root, const char* suffix) {
  int len = strlen(suffix);
  for (int i = 0; i < len; ++i) {
    Node* current = root;
    for (int j = i; suffix[j] != '\0'; j++) {
      int idx = suffix[j];
      if (current->children[idx] == NULL) {
        current->children[idx] = createNode();
      }
      current = current->children[idx];
    }
    current->isEndOfWord = 1;
  }
}

void display(Node* root, char str[], int level) {
  if (root->isEndOfWord) {
    str[level] = '\0';
    printf("%s\n", str);
  }
  for (int i = 0; i < CHAR_SIZE; i++) {
    if (root->children[i] != NULL) {
      str[level] = i;
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
  return 1;
}

int countFileLines(FILE* fp) {
  if (fp == NULL) {
    fprintf(stderr, "fopen() failed\n");
    return -1;
  }

  int count = 0;

  char ch;
  while (!feof(fp)) {
    ch = fgetc(fp);
    if (ch == '\n') {
      count++;
    }
  }

  fseek(fp, 0, SEEK_SET);
  return count;
}

int countLine(FILE* fp) {
  if (fp == NULL) {
    fprintf(stderr, "fopen() failed\n");
    return -1;
  }
  int count = 0;

  char* line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, fp)) != -1) {
    printf("%s [%lu]\n", line, strlen(line));
    count++;
  }

  fseek(fp, 0, SEEK_SET);
  free(line);
  return count;
}

Keyword* createKeyword(FILE* fp, int lines) {
  if (fp == NULL) {
    fprintf(stderr, "fopen() failed\n");
    return NULL;
  }

  Keyword* keywords = (Keyword*)malloc(sizeof(Keyword) * lines);
  if (keywords == NULL) {
    perror("malloc() failed");
    return NULL;
  }

  char* line = NULL;
  size_t len = 0;
  ssize_t read;

  int i = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    int j = 0;
    int idxOfLastSpace = -1;
    // 공백
    while (line[j] != '\0') {
      if (line[j] == ' ') {
        idxOfLastSpace = j;
      }
      j++;
    }

    char* readCountStr = line + idxOfLastSpace + 1;
    line[idxOfLastSpace] = '\0';
    char* word = strdup(line);

    keywords[i].word = word;
    keywords[i].wordLength = strlen(line) + 1;
    keywords[i].searchCount = atoi(readCountStr);

    keywords[i].trieRoot = createNode();
    insertSuffix(keywords[i].trieRoot, keywords[i].word);

    i++;
  }

  fseek(fp, 0, SEEK_SET);
  return keywords;
}

void printKeywords(Keyword* keywords, int n) {
  for (int i = 0; i < n; i++) {
    printf("[%d]%s,%d,isPohang=%d\n", keywords[i].searchCount, keywords[i].word,
           keywords[i].wordLength, search(keywords[i].trieRoot, "Pohang"));
  }
}

ssize_t readn(int fd, void* data, size_t n) {
  size_t left;
  size_t read_n;

  left = n;

  while (left > 0) {
    if ((read_n = read(fd, data, left)) < 0) {
      if (left == n)
        return -1;
      else
        break;
    } else if (read_n == 0)
      break;
    left -= read_n;
    data += read_n;
  }

  return n - left;
}

ssize_t writen(int fd, const void* data, size_t n) {
  size_t left;
  ssize_t written_n;

  left = n;

  while (left > 0) {
    if ((written_n = write(fd, data, left)) < 0) {
      if (left == n)
        return -1;
      else
        break;
    } else if (written_n == 0)
      break;
    left -= written_n;
    data += written_n;
  }

  return n - left;
}

void errorExit(const char* err) {
  perror(err);
  exit(1);
}