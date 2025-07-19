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

void setPacket(Packet *packet, void *data) {
  memcpy(&packet->data, data, sizeof(*data));
}

int r_sendto(ConnectionInfo *conn, void *data_buff, unsigned int dataSize,
             unsigned int curr_seq) {
  int rw_len = 0;

  Packet sendPacket, recvPacket;
  initPacket(&sendPacket);
  setPacketHeader(&sendPacket, PKT_DATA, curr_seq, 0, dataSize);
  setPacket(&sendPacket, data_buff);

  struct timeval timeoutTime = {0, 500000};  // 0.5sec

  struct sockaddr_in from;
  socklen_t from_len = sizeof(from);

  if (setsockopt(conn->sock, SOL_SOCKET, SO_RCVTIMEO, &timeoutTime,
                 sizeof(timeoutTime)) == -1) {
    perror("setsockopt() failed");
    return -1;
  }

  int try = 0;
  while (try < MAX_REQ) {
    rw_len = sendto(conn->sock, &sendPacket, PKT_SIZE, 0, &conn->recv_addr,
                    conn->recv_addr_len);
    if (rw_len == -1) {
      perror("sendto() failed");
      continue;
    }

    rw_len =
        recvfrom(conn->sock, &recvPacket, PKT_SIZE, 0,
                 (struct sockaddr *)&conn->recv_addr, &conn->recv_addr_len);
    if (rw_len == -1) {
      if (errno == EAGAIN) {
        try++;
        continue;
      } else {
        perror("recvfrom() failed");
        return -1;
      }
    }

    if (recvPacket.header.packetType == PKT_ACK && recvPacket.header.ack == 1 &&
        recvPacket.header.seq == curr_seq) {
      // 전송 성공 시 timeout off
      timeoutTime.tv_usec = 0;
      if (setsockopt(conn->sock, SOL_SOCKET, SO_RCVTIMEO, &timeoutTime,
                     sizeof(timeoutTime)) == -1) {
        perror("setsockopt() failed");
        return -1;
      }
      return 0;
    }

    try++;
  }  // while

  fprintf(stderr, "r_sendto() failed\n");
  return -1;
}

int r_recvfrom(ConnectionInfo *conn, void *data_buff, unsigned int curr_seq) {
  int rw_len = 0;

  Packet recvPacket, ackPacket;
  initPacket(&ackPacket);
  setPacketHeader(&ackPacket, PKT_ACK, curr_seq, 1, 0);

  struct timeval timeoutTime = {0, 500000};  // 0.5sec

  struct sockaddr_in from_addr;
  socklen_t from_addr_len = sizeof(from_addr);

  if (setsockopt(conn->sock, SOL_SOCKET, SO_RCVTIMEO, &timeoutTime,
                 sizeof(timeoutTime)) == -1) {
    perror("setsockopt() failed");
    return -1;
  }

  int try = 0;
  while (try < MAX_REQ) {
    rw_len = recvfrom(conn->sock, &recvPacket, PKT_SIZE, 0,
                      (struct sockaddr *)&from_addr, &from_addr_len);
    if (rw_len == -1) {
      if (errno == EAGAIN) {
        try++;
        continue;
      } else {
        perror("recvfrom() failed");
        return -1;
      }
    }

    if (recvPacket.header.packetType == PKT_DATA) {
      if (recvPacket.header.seq == curr_seq) {  // seq 같으면 데이터 복사
        // data_buff에 packet data 복사
        memcpy(data_buff, recvPacket.data, recvPacket.header.dataSize);

        sendto(conn->sock, &ackPacket, PKT_SIZE, 0,
               (struct sockaddr *)&conn->recv_addr, &conn->recv_addr_len);

        timeoutTime.tv_usec = 0;
        if (setsockopt(conn->sock, SOL_SOCKET, SO_RCVTIMEO, &timeoutTime,
                       sizeof(timeoutTime)) == -1) {
          perror("setsockopt() failed");
          return -1;
        }

        return recvPacket.header.dataSize;
      } else {  // seq 다르면 ack 재전송]
        sendto(conn->sock, &ackPacket, PKT_SIZE, 0,
               (struct sockaddr *)&conn->recv_addr, &conn->recv_addr_len);
        continue;
      }
    }

    try++;
  }  // while

  fprintf(stderr, "r_recvfrom() failed\n");
  return -1;
}

void timeout(int sig) {
  if (sig == SIGALRM) {
    timeoutFlag = 1;
#ifdef DEBUG
    puts("TIMEOUT");
#endif
  }
}