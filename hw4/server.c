#include "console.h"
#include "search.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <port> <datafile>\n");
    exit(1);
  }

  FILE *dataFile = fopen(argv[2], "r");
  if (dataFile == NULL) {
    perror("fopen() failed");
    exit(1);
  }

  fclose(dataFile);
  return 0;
}