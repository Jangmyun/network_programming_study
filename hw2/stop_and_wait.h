#ifndef STOP_AND_WAIT_H
#define STOP_AND_WAIT_H

#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PKT_SIZE 512

typedef struct {  // 12 bytes
  unsigned int seq;
  unsigned int ack;
  unsigned int data_size;
} pkt_t_h;

#define PKT_DATA_SIZE (PKT_SIZE - sizeof(pkt_t_h))  // 500 bytes

typedef struct {
  pkt_t_h header;
  char data[PKT_DATA_SIZE];
} pkt_t;

#endif  // STOP_AND_WAIT_H