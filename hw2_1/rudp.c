#include "rudp.h"

int timeoutFlag = 0;

void initPacket(Packet *packet) { memset(packet, 0, sizeof(Packet)); }

void initPacketHeader(Packet *packet) {
  memset(&packet->header, 0, sizeof(PacketHeader));
}

void setPacketHeader(Packet *packet, unsigned short packetType,
                     unsigned int seq, unsigned short ack,
                     unsigned int dataSize) {
  packet->header.packetType = packetType;
  packet->header.seq = seq;
  packet->header.ack = ack;
  packet->header.dataSize = dataSize;
}

void setPacket(Packet *packet, char *data) {
  memcpy(&packet->data, data, sizeof(*data));
}

void timeout(int sig) {
  if (sig == SIGALRM) {
    timeoutFlag = 1;
#ifdef DEBUG
    puts("TIMEOUT");
#endif
  }
}