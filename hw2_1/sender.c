#include "rudp.h"

int setConnectionInfo(ConnectionInfo *conn, char *portArg);
void connectWithClient(ConnectionInfo *conn);
void sendFile(ConnectionInfo *conn, char *filename);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <port> <filename>\n", argv[0]);
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
  }

  signal(SIGALRM, timeout);

  connectWithClient(&conn);

  sendFile(&conn, argv[2]);

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

void connectWithClient(ConnectionInfo *conn) {
  Packet sendPacket, recvPacket;
  setPacketHeader(&sendPacket, PKT_CONNECTION_REQ, 0, 1, 0);

  struct sockaddr_in from_addr;
  socklen_t from_addr_len = sizeof(from_addr);

  int rw_len = 0;
  puts("Connection Waiting ...");

  int connectionTry = 0;
  while (connectionTry < MAX_REQ) {
    rw_len = recvfrom(conn->sock, &recvPacket, PKT_SIZE, 0,
                      (struct sockaddr *)&from_addr, &from_addr_len);
    if (rw_len == -1) {
      perror("recvfrom() failed");
      connectionTry++;
      continue;
    }

    if (recvPacket.header.packetType == PKT_CONNECTION_REQ) {
#ifdef DEBUG
      puts("Connection request received");
#endif

      rw_len = sendto(conn->sock, &sendPacket, PKT_SIZE, 0,
                      (struct sockaddr *)&from_addr, from_addr_len);
      if (rw_len == -1) {
        perror("sendto() failed");
        connectionTry++;
        continue;
      }
      conn->recv_addr = from_addr;
      conn->recv_addr_len = from_addr_len;

      break;
    }

    connectionTry++;
  }

  if (connectionTry >= MAX_REQ) {
    fprintf(stderr, "Connection Failed\n");
    close(conn->sock);
    exit(1);
  }

  puts("Connection Established");
  return;
}

void sendFile(ConnectionInfo *conn, char *filename) {
  int rw_len;
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror("fopen() failed");
    exit(1);
  }

  // filename 전송
  rw_len = r_sendto(conn, filename, strlen(filename) + 1, 0);
  if (rw_len == -1) {
    fclose(fp);
    return;
  }

  // filesize 전송
  fseek(fp, 0, SEEK_END);
  unsigned int filesize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  rw_len = r_sendto(conn, &filesize, sizeof(unsigned int), 0);
  if (rw_len == -1) {
    fclose(fp);
    return;
  }

  // file 전송
  unsigned int send_size = 0;
  char buf[PKT_DATA_SIZE];
  int seq = 0;

  while (1) {
    rw_len = fread((void *)buf, 1, PKT_DATA_SIZE, fp);
    if (rw_len < PKT_DATA_SIZE) {
      if (r_sendto(conn, buf, rw_len, seq) == -1) {
        fclose(fp);
        return;
      }
      seq++;
      break;
    }
    if (r_sendto(conn, buf, PKT_DATA_SIZE, seq) == -1) {
      fclose(fp);
      return;
    }
    seq++;
  }

  fclose(fp);
  puts("File sent successfully");
}
