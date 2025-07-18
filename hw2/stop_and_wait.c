#include "stop_and_wait.h"

int timeout_flag = 0;
unsigned int seq = 0;

int init_packet(pkt_t *p) {
  memset(p, 0, sizeof(*p));
  return 0;
}

int init_packet_header(pkt_t_h *ph) {
  memset(ph, 0, sizeof(*ph));
  return 0;
}

void set_packet_header(pkt_t_h *ph, unsigned short pkt_type, unsigned int seq,
                       unsigned short ack, unsigned int data_size) {
  ph->pkt_type = pkt_type;
  ph->seq = seq;
  ph->ack = ack;
  ph->data_size = data_size;
}
void set_packet(pkt_t *p, char *data) {
  // header 사이즈만큼 pointer 이동 후 data에만 memcpy
  memcpy(p + sizeof(pkt_t_h), data, PKT_DATA_SIZE);
}

void timeout(int sig) {
  if (sig == SIGALRM) {
#ifdef DEBUG
    printf("timeout\n");
#endif
    timeout_flag = 1;
  }
}

ssize_t reliable_sendto(int sock, void *buff, size_t nbytes, int flags,
                        struct sockaddr *to, struct sockaddr *connected_addr,
                        unsigned int curr_seq) {
  ssize_t read_len = 0;
  pkt_t recv_pkt;

  struct sockaddr_in from;
  socklen_t from_len = sizeof(from);

  int req_time = 0;
  while (req_time < MAX_REQ) {
    sendto(sock, buff, nbytes, flags, to, sizeof(*to));

    ualarm(500000, 0);
    timeout_flag = 0;

    read_len = recvfrom(sock, &recv_pkt, PKT_SIZE, 0, (struct sockaddr *)&from,
                        &from_len);
    if (read_len == -1) {
      perror("recvfrom() failed");
      req_time++;
      continue;
    }

    // connected 된 address와 recvfrom으로 받은 address가 다르면 무시
    if (((struct sockaddr_in *)connected_addr)->sin_addr.s_addr !=
        from.sin_addr.s_addr) {
      req_time++;
      continue;
    }

    // ack packet을 정상적으로 받았으면 읽은 사이즈 리턴
    if (recv_pkt.header.pkt_type == TYPE_ACK && recv_pkt.header.ack == 1 &&
        recv_pkt.header.seq == curr_seq) {
      ualarm(0, 0);
      timeout_flag = 0;
      return read_len;
    }
  }

  return -1;
}

