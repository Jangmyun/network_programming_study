#include "stop_and_wait.h"

void timeout(int sig);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int serv_sock = 0;
  char buf[PKT_SIZE];
  int len = 0;
  socklen_t clnt_addr_size;

  return 0;
}

void timeout(int sig) {
  if (sig != SIGALRM) return;
}