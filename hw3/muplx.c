#include "muplx.h"

void errorExit(const char *err) {
  perror(err);
  exit(1);
}

ssize_t readn(int fd, void *data, size_t n) {
  size_t left;
  size_t read_n;

  left = n;

  while (left > 0) {
    if ((read_n = read(fd, data, left)) < 0) {
      if (left == n)
        return -1;
      else
        break;
    } else if (read_n == 0)
      break;
    left -= read_n;
    data += read_n;
  }

  return n - left;
}

ssize_t writen(int fd, const void *data, size_t n) {
  size_t left;
  ssize_t written_n;

  left = n;

  while (left > 0) {
    if ((written_n = write(fd, data, left)) < 0) {
      if (left == n)
        return -1;
      else
        break;
    } else if (written_n == 0)
      break;
    left -= written_n;
    data += written_n;
  }

  return n - left;
}

int count_files(char *path) {
  DIR *dir = NULL;
  struct dirent *entry;

  if (path == NULL) return -1;
  if ((dir = opendir(path)) == NULL) return -1;

  int count = 0;

  while ((entry = readdir(dir))) {
    // file 혹은 디렉토리면 count증가
    if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
      count++;
    }
  }

  closedir(dir);

  return count;
}

FileInfo *readFiles(char *path, int *file_num) {
  int count = 0;
  int filecount = count_files(path);
  if (path == NULL || filecount < 1) exit(1);

  FileInfo *fileinfos = (FileInfo *)malloc(sizeof(FileInfo) * filecount);
  if (fileinfos == NULL) {
    errorExit("malloc() failed");
  }

  *file_num = filecount;

  DIR *dir = NULL;
  struct dirent *entry;
  struct stat curr_stat;

  dir = opendir(path);
  if (dir == NULL) {
    errorExit("opendir() failed");
  }

  while ((entry = readdir(dir))) {
    if (count == filecount) break;
    if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
      stat(entry->d_name, &curr_stat);
      strncpy(fileinfos[count].filename, entry->d_name, MAX_FILENAME_SIZE);
      fileinfos[count].filename[MAX_FILENAME_SIZE] = 0;
      fileinfos[count].size = curr_stat.st_size;

      int isDir = entry->d_type == DT_DIR ? 1 : 0;
      fileinfos[count].isDir = isDir;

      count++;
    }
  }

  closedir(dir);
  return fileinfos;
}

void sendCwdInfos(int clnt_sock) {
  // 현재 디렉토리 정보 전송
  char currentPathname[PATH_MAX];
  getcwd(currentPathname, PATH_MAX);

  writen(clnt_sock, currentPathname, PATH_MAX);

  // 현재 디렉토리의 파일 개수 전송
  int fileCount = 0;
  FileInfo *fileInfos = readFiles(".", &fileCount);

  writen(clnt_sock, &fileCount, sizeof(int));

  for (int i = 0; i < fileCount; i++) {
    writen(clnt_sock, &fileInfos[i], sizeof(FileInfo));

    printf("[SEND FILEINFO] filename=%s | size=%ld | isDir=%d\n",
           fileInfos[i].filename, fileInfos[i].size, fileInfos[i].isDir);
  }
}

void receiveCwdInfos(int serv_sock) {
  char currentPathname[PATH_MAX];
  readn(serv_sock, currentPathname, PATH_MAX);

  printf("<CURRENT DIRECTORY : %s>\n", currentPathname);

  int fileCount = 0;
  readn(serv_sock, &fileCount, sizeof(int));

  FileInfo recvFileInfo;
  for (int i = 0; i < fileCount; i++) {
    readn(serv_sock, &recvFileInfo, sizeof(FileInfo));

    printf("%d %6d %s\n", recvFileInfo.isDir, recvFileInfo.size,
           recvFileInfo.filename);
  }
}