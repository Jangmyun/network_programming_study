#include "stop_and_wait.h"

void timeout(int sig);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int sender_sock = 0;
  char buf[PKT_SIZE];
  int read_cnt = 0;
  socklen_t clnt_addr_size;

  pkt_t_h packet_header;
  pkt_t packet;

  sender_sock = socket(PF_INET, SOCK_DGRAM, 0);
  if (sender_sock == -1) {
    perror("socket() failed");
    exit(1);
  }

  // set REUSEADDR socket option
  int optval = 1;
  if (setsockopt(sender_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                 sizeof(optval))) {
    perror("setsockopt() error");
    exit(1);
  }

  struct sockaddr_in sender_addr, receiver_addr;
  struct sockaddr_in curr_receiver_addr;

  memset(&sender_addr, 0, sizeof(sender_addr));
  sender_addr.sin_family = AF_INET;
  sender_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sender_addr.sin_port = htons(atoi(argv[1]));

  if (bind(sender_sock, (struct sockaddr *)&sender_addr, sizeof(sender_addr)) ==
      -1) {
    perror("bind() failed");
    exit(1);
  }

  return 0;
}
