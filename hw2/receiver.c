#include "stop_and_wait.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <IP> <port>\n", argv[0]);
    exit(1);
  }

  int sock;
  char buf[PKT_SIZE] = "";
  int rw_len;
  socklen_t addr_size = sizeof(struct sockaddr_in);

  struct sockaddr_in sender_addr, from_addr;

  sock = socket(PF_INET, SOCK_DGRAM, 0);
  if (sock == -1) {
    perror("socket() failed");
    exit(1);
  }

  memset(&sender_addr, 0, sizeof(sender_addr));
  sender_addr.sin_family = AF_INET;
  sender_addr.sin_addr.s_addr = inet_addr(argv[1]);
  sender_addr.sin_port = htons(atoi(argv[2]));

  signal(SIGALRM, timeout);

  pkt_t send_pkt, recv_pkt;
  pkt_t received_pkt;
  init_packet(&send_pkt);

  send_pkt.header.pkt_type = TYPE_CONNECTION_REQ;
  set_packet(&send_pkt, buf);

  int req_time = 0;

  // connection req
  while (req_time < MAX_REQ) {
    sendto(sock, &send_pkt, PKT_SIZE, 0, (struct sockaddr *)&sender_addr,
           sizeof(sender_addr));

#ifdef DEBUG
    printf("Request sent\n");
#endif

    // set alarm & init timeout_flag
    ualarm(500000, 0);
    timeout_flag = 0;

    rw_len = recvfrom(sock, &recv_pkt, PKT_SIZE, 0,
                      (struct sockaddr *)&from_addr, &addr_size);
    if (rw_len == -1) {
      perror("read() failed");
      req_time++;
      continue;
    }

    // 다른 주소에서 온 패킷 처리
    if (sender_addr.sin_addr.s_addr != from_addr.sin_addr.s_addr) {
      req_time++;
      continue;
    }

    // CONNECTION_REQ에 대한 ACK를 수신하면
    if (recv_pkt.header.pkt_type == TYPE_CONNECTION_REQ &&
        recv_pkt.header.ack) {
      ualarm(0, 0);
      timeout_flag = 0;
      break;
    }

#ifdef DEBUG
    printf("Req failed\n");
#endif
  }  // while

  // connection failed
  if (timeout_flag == 1) {
    printf("Connection failed\n");
    close(sock);
    return 0;
  }

  printf("Connection success\n");

  // file 받기

  printf("Receiver Terminated\n");

  close(sock);
  return 0;
}