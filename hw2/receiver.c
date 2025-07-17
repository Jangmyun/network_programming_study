#include "stop_and_wait.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <IP> <port>\n", argv[0]);
    exit(1);
  }

  int sock;
  char buf[PKT_SIZE] = "";
  int read_cnt;
  socklen_t addr_size;

  struct sockaddr_in sender_addr;

  sock = socket(PF_INET, SOCK_DGRAM, 0);
  if (sock == -1) {
    perror("socket() failed");
    exit(1);
  }

  memset(&sender_addr, 0, sizeof(sender_addr));
  sender_addr.sin_family = AF_INET;
  sender_addr.sin_addr.s_addr = inet_addr(argv[1]);
  sender_addr.sin_port = htons(atoi(argv[2]));

  // connected sock
  if (connect(sock, (struct sockaddr *)&sender_addr, sizeof(sender_addr)) ==
      -1) {
    perror("connect() failed");
    exit(1);
  }

  pkt_t pkt;
  pkt_t received_pkt;
  init_packet(&pkt);

  // connection req
  pkt.header.pkt_type = TYPE_CONNECTION_REQ;
  set_packet(&pkt, buf);

  signal(SIGALRM, timeout);
  while (1) {
    alarm(2);
    write(sock, &pkt, PKT_SIZE);
    read(sock, &received_pkt, PKT_SIZE);

    if (received_pkt.header.pkt_type == TYPE_CONNECTION_REQ &&
        received_pkt.header.ack) {
      break;
    }
  }
  alarm(0);

  return 0;
}