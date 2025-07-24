#include "console.h"
#include "search.h"

typedef struct {
  int sock;
  fd_set *readfds;
} ClientInfo;

void *keywordHandler(void *arg);

pthread_mutex_t fdSetMutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <port> <datafile>\n", argv[0]);
    exit(1);
  }

  FILE *dataFile = fopen(argv[2], "r");
  if (dataFile == NULL) {
    perror("fopen() failed");
    exit(1);
  }

  int dataLine = countFileLines(dataFile);

  Keyword *keywords = createKeyword(dataFile, dataLine);
  puts("Data Ready!");

#ifdef DEBUG
  printKeywords(keywords, dataLine);
#endif

  int serv_sock, clnt_sock;
  struct sockaddr_in serv_addr, clnt_addr;

  socklen_t addr_size = sizeof(struct sockaddr_in);

  int fd_max, fd_num;
  int rw_len;
  char buf[BUF_SIZE];

  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  int optval = 1;
  if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&optval,
                 sizeof(optval))) {
    errorExit("setsockopt() error");
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
    errorExit("bind() error");
  }

  if (listen(serv_sock, 5) == -1) {
    errorExit("listen() error");
  }

  fd_set readfds, tempReads;

  FD_ZERO(&readfds);
  FD_SET(serv_sock, &readfds);

  puts("Server On!");

  while (1) {
    tempReads = readfds;

    fd_num = select(fd_max + 1, &tempReads, 0, 0, NULL);
    if (fd_num == -1) break;
    if (fd_num == 0) continue;

    for (int sd = 0; sd <= fd_max; sd++) {
      if (FD_ISSET(sd, &readfds)) {  // readfds event
        if (sd == serv_sock) {       // server socket event
          addr_size = sizeof(clnt_addr);
          clnt_sock =
              accept(serv_sock, (struct sockaddr *)&clnt_addr, &addr_size);
          if (fd_max < clnt_sock) fd_max = clnt_sock;
          printf("Client %d accepted\n", clnt_sock);

          pthread_mutex_lock(&fdSetMutex);
          FD_SET(clnt_sock, &readfds);
          pthread_mutex_unlock(&fdSetMutex);

        } else {  // client socket event
          ClientInfo *clntInfo = (ClientInfo *)malloc(sizeof(ClientInfo));
          if (clntInfo == NULL) {
            perror("malloc() failed");
            close(clnt_sock);
            continue;
          }
          clntInfo->readfds = &readfds;
          clntInfo->sock = clnt_sock;

          pthread_t tid;
          int ret =
              pthread_create(&tid, NULL, keywordHandler, (void *)clntInfo);
          if (ret != 0) {
            fprintf(stderr, "pthread_create() failed %s\n", strerror(ret));
            close(clnt_sock);
            continue;
          }

          pthread_detach(tid);
        }
      }
    }
  }

  fclose(dataFile);
  return 0;
}

void *keywordHandler(void *arg) {
  ClientInfo clntInfo = *(ClientInfo *)arg;
  int sock = clntInfo.sock;

  pthread_mutex_lock(&fdSetMutex);
  FD_CLR(clntInfo.sock, clntInfo.readfds);
  pthread_mutex_unlock(&fdSetMutex);

  pthread_mutex_lock(&fdSetMutex);
  FD_SET(clntInfo.sock, clntInfo.readfds);
  pthread_mutex_unlock(&fdSetMutex);
  return NULL;
}