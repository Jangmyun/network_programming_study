#include "stop_and_wait.h"

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
void set_packet(pkt_t *p, pkt_t_h *ph, char *data) {
  memcpy(p, ph, sizeof(*ph));
  // ph만큼 pointer 이동 후 data에만 memcpy
  memcpy(p + sizeof(*ph), data, ph->data_size);
}