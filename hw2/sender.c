#include "stop_and_wait.h"

void timeout(int sig);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int sender_sock = 0;
  char buf[PKT_SIZE];
  int rw_len = 0;
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
  struct sockaddr_in from_addr;

  socklen_t addr_len = sizeof(from_addr);

  memset(&sender_addr, 0, sizeof(sender_addr));
  sender_addr.sin_family = AF_INET;
  sender_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sender_addr.sin_port = htons(atoi(argv[1]));

  if (bind(sender_sock, (struct sockaddr *)&sender_addr, sizeof(sender_addr)) ==
      -1) {
    perror("bind() failed");
    exit(1);
  }

  signal(SIGALRM, timeout);
  pkt_t send_pkt, recv_pkt;
  init_packet(&send_pkt);

  send_pkt.header.pkt_type = TYPE_CONNECTION_REQ;
  set_packet(&send_pkt, buf);

  printf("Connection Waiting...\n");
  // connection res
  while (1) {
#ifdef DEBUG
    printf("sender is ready to recv req");
#endif
    rw_len = recvfrom(sender_sock, &recv_pkt, PKT_SIZE, 0,
                      (struct sockaddr *)&from_addr, &addr_len);
    if (rw_len == -1) {
      perror("recvfrom() failed");
      continue;
    }

    if (recv_pkt.header.pkt_type == TYPE_CONNECTION_REQ) {
#ifdef DEBUG
      printf("Connection request received\n");
#endif
      set_packet_header(&send_pkt.header, TYPE_CONNECTION_REQ, 0, 1, 0);

      sendto(sender_sock, &send_pkt, PKT_SIZE, 0,
             (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));
      break;
    }
  }
  printf("Connection success\n");

  // file 보내기

  close(sender_sock);
  return 0;
}
