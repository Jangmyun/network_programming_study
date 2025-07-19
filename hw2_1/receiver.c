#include "rudp.h"

int setConnectionInfo(ConnectionInfo *conn, char *ipArg, char *portArg);
void connectWithServer(ConnectionInfo *conn);

int main(int argc, char *argv[]) {
  ConnectionInfo conn;
  char buf[PKT_DATA_SIZE];
  int rw_len;

  setConnectionInfo(&conn, argv[1], argv[2]);

  signal(SIGALRM, timeout);
  struct sockaddr_in from_addr;

  connectWithServer(&conn);

  return 0;
}

int setConnectionInfo(ConnectionInfo *conn, char *ipArg, char *portArg) {
  conn->sock = socket(PF_INET, SOCK_DGRAM, 0);
  if (conn->sock == -1) {
    perror("socket() failed");
    exit(1);
  }

  memset(&conn->addr, 0, sizeof(conn->addr));
  conn->addr.sin_family = AF_INET;
  conn->addr.sin_addr.s_addr = inet_addr(ipArg);
  conn->addr.sin_port = htons(atoi(portArg));

  conn->addr_len = sizeof(struct sockaddr_in);

  return 0;
}

void connectWithServer(ConnectionInfo *conn) {
  Packet sendPacket, recvPacket;
  setPacketHeader(&sendPacket, PKT_CONNECTION_REQ, 0, 0, 0);

  struct sockaddr_in from_addr;
  socklen_t from_addr_len = sizeof(from_addr);

  int rw_len = 0;
  puts("Connect Request Sent ...");

  int connectionTry = 0;
  while (connectionTry < MAX_REQ) {
    rw_len = sendto(conn->sock, &sendPacket, PKT_SIZE, 0,
                    (struct sockaddr *)&conn->addr, conn->addr_len);
    if (rw_len == -1) {
      perror("sendto() failed");
      connectionTry++;
      continue;
    }
#ifdef DEBUG
    puts("Request sent");
#endif

    ualarm(500000, 0);
    timeoutFlag = 0;

    rw_len = recvfrom(conn->sock, &recvPacket, PKT_SIZE, 0,
                      (struct sockaddr *)&from_addr, &from_addr_len);
    if (rw_len == -1) {
      perror("read() failed");
      connectionTry++;
      continue;
    }

    if (conn->addr.sin_addr.s_addr != from_addr.sin_addr.s_addr) {
      connectionTry++;
      continue;
    }

    if (recvPacket.header.packetType == PKT_CONNECTION_REQ &&
        recvPacket.header.ack == 1) {
      ualarm(0, 0);
      timeoutFlag = 0;
      break;
    }

  }  // while

  if (timeoutFlag == 1 || connectionTry >= MAX_REQ) {
    puts("Connection failed");
    close(conn->sock);
    exit(1);
  }

  puts("Connection Established");
  return;
}
