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
  memcpy(p->data, data, PKT_DATA_SIZE);
}

void set_data(pkt_t *p, char *data, unsigned int data_size) {
  memcpy(p->data, data, PKT_DATA_SIZE);
  p->header.data_size = data_size;
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

ssize_t reliable_recvfrom(int sock, void *buff, size_t nbytes, int flags,
                          struct sockaddr *from, socklen_t *addrlen,
                          unsigned int curr_seq) {
  ssize_t read_len = 0;
  pkt_t recv_pkt;

  while (1) {
    read_len = recvfrom(sock, &recv_pkt, PKT_SIZE, flags, from, addrlen);
    if (read_len == -1) {
      perror("recvfrom() failed");
      return -1;
    }

    pkt_t ack_pkt;
    init_packet(&ack_pkt);
    set_packet_header(&ack_pkt.header, TYPE_ACK, recv_pkt.header.seq, 1, 0);

    if (recv_pkt.header.pkt_type == TYPE_DATA) {
#ifdef DEBUG
      printf("header_seq=%d | curr_seq=%d\n", recv_pkt.header.seq, curr_seq);
#endif

      // seq 같으면 valid data 이므로 memcpy
      // seq 다르면 이미 받아온 data이므로 ack만 다시 보내기
      if (recv_pkt.header.seq == curr_seq) {
        memcpy(buff, &recv_pkt, PKT_SIZE);
        sendto(sock, &ack_pkt, PKT_SIZE, 0, from, *addrlen);
        return PKT_SIZE;
      } else {
        sendto(sock, &ack_pkt, PKT_SIZE, 0, from, *addrlen);
        continue;
      }
    }
  }

  return -1;
}
