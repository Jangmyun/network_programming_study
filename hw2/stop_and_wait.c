#include "stop_and_wait.h"

int init_packet(pkt_t *p) {
  memset(p, 0, sizeof(*p));
  return 0;
}

int init_packet_header(pkt_t_h *ph) {
  memset(ph, 0, sizeof(*ph));
  return 0;
}

