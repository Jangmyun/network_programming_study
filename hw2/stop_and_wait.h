#ifndef STOP_AND_WAIT_H
#define STOP_AND_WAIT_H

#include <arpa/inet.h>
#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PKT_SIZE 512

// pkt_type
#define TYPE_DATA 0
#define TYPE_ACK 1
#define TYPE_CONNECTION_REQ 2
#define TYPE_CONNECTION_CLOSE 3

extern int timeout_flag;

typedef struct {
  unsigned short pkt_type;  // data, ack, connection_req
  unsigned int seq;
  unsigned short ack;
  unsigned int data_size;
} pkt_t_h;

#define PKT_DATA_SIZE (PKT_SIZE - sizeof(pkt_t_h))  // 500 bytes

typedef struct {
  pkt_t_h header;
  char data[PKT_DATA_SIZE];
} pkt_t;

// init functions
int init_packet(pkt_t *p);
int init_packet_header(pkt_t_h *ph);

// packet setter functions
void set_packet_header(pkt_t_h *ph, unsigned short pkt_type, unsigned int seq,
                       unsigned short ack, unsigned int data_size);
void set_packet(pkt_t *p, char *data);

// alarm handler
void timeout(int sig);

#endif  // STOP_AND_WAIT_H