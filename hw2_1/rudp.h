#ifndef RUDP_H
#define RUDP_H

#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_REQ 20

// PACKET_TYPE
#define PKT_DATA 0
#define PKT_ACK 0
#define PKT_CONNECTION_REQ 0

extern int timeoutFlag;

typedef struct {
  unsigned short packetType;
  unsigned int seq;
  unsigned short ack;
  unsigned int dataSize;
} PacketHeader;

#define PKT_SIZE 1024
#define PKT_DATA_SIZE (PKT_SIZE - sizeof(PacketHeader))

typedef struct {
  PacketHeader header;
  char data[PKT_DATA_SIZE];
} Packet;

typedef struct {
  int sock;  // socket descriptor
  struct sockaddr_in addr;
  socklen_t addr_len;
  struct sockaddr_in recv_addr;  // unused for client
  socklen_t recv_addr_len;       // unused for client
} ConnectionInfo;

void initPacket(Packet *packet);

void initPacketHeader(Packet *packet);

void setPacketHeader(Packet *packet, unsigned short packetType,
                     unsigned int seq, unsigned short ack,
                     unsigned int dataSize);
void setPacket(Packet *packet, char *data);

void timeout(int sig);

#endif