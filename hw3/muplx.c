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

FileInfo *read_files(char *path) {
  int count = 0;
  int filecount = count_files(path);
  if (path == NULL || filecount < 1) exit(1);

  FileInfo *fileinfos = (FileInfo *)malloc(sizeof(FileInfo) * filecount);
  if (fileinfos == NULL) {
    errorExit("malloc() failed");
  }

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