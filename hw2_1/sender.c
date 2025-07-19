#include "rudp.h"

int setConnectionInfo(ConnectionInfo *conn, char *portArg);
void waitConnection(ConnectionInfo *conn);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  ConnectionInfo conn;
  char buf[PKT_SIZE];
  int rw_len = 0;

  setConnectionInfo(&conn, argv[1]);

  struct sockaddr_in from_addr;  // 정보 받아올 address;
  socklen_t from_addr_size;

  if (bind(conn.sock, (struct sockaddr *)&conn.addr, sizeof(struct sockaddr)) ==
      -1) {
    perror("bind() failed");
    exit(1);

    signal(SIGALRM, timeout);
    Packet send_packet, recv_packet;
    initPacket(&send_packet);

    send_packet.header.packetType = PKT_CONNECTION_REQ;
  }

  return 0;
}

int setConnectionInfo(ConnectionInfo *conn, char *portArg) {
  conn->sock = socket(PF_INET, SOCK_DGRAM, 0);
  if (conn->sock == -1) {
    perror("socket() failed");
    exit(1);
  }

  // set REUSEADDR socket option
  int optval = 1;
  if (setsockopt(conn->sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                 sizeof(optval))) {
    perror("setsockopt() error");
    exit(1);
  }

  conn->addr_len = sizeof(struct sockaddr_in);
  conn->recv_addr_len = sizeof(struct sockaddr_in);

  memset(&conn->addr, 0, sizeof(conn->addr));
  conn->addr.sin_family = AF_INET;
  conn->addr.sin_addr.s_addr = htonl(INADDR_ANY);
  conn->addr.sin_port = htons(atoi(portArg));

  return 1;
}

void waitConnection(ConnectionInfo *conn) {
  Packet sendPacket, recvPacket;
  setPacketHeader(&sendPacket, PKT_CONNECTION_REQ, 0, 1, 0);

  struct sockaddr_in from_addr;
  socklen_t from_addr_len = sizeof(from_addr);

  int rw_len = 0;
  puts("Connection Waiting ...");

  int connectionTry = 0;
  while (1) {
    rw_len = recvfrom(conn->sock, &recvPacket, PKT_SIZE, 0,
                      (struct sockaddr *)&from_addr, &from_addr_len);
    if (rw_len == -1) {
      perror("recvfrom() failed");
      connectionTry++;
      continue;
    }
  }
  return;
}