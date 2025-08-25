#include "file_info.h"

// path 디렉토리의 파일 개수를 카운트하고, filename_size set
int count_files(char *path) {
  DIR *dir = NULL;
  struct dirent *entry;

  if (path == NULL) return -1;
  if ((dir = opendir(path)) == NULL) return -1;

  int count = 0;

  while ((entry = readdir(dir))) {
    // file이면 count++
    if (entry->d_type == DT_REG) {
      count++;
    }
  }

  closedir(dir);

  return count;
}

FileInfo *read_files(char *path, int filecount) {
  int count = 0;
  if (path == NULL || filecount < 1) exit(1);

  FileInfo *fileinfos = (FileInfo *)malloc(sizeof(FileInfo) * filecount);
  if (fileinfos == NULL) {
    perror("malloc() failed");
    exit(1);
  }

  DIR *dir = NULL;
  struct dirent *entry;
  struct stat curr_stat;

  dir = opendir(path);
  if (dir == NULL) {
    perror("opendir() failed");
    exit(1);
  }

  while ((entry = readdir(dir))) {
    if (count == filecount) break;
    if (entry->d_type == DT_REG) {
      stat(entry->d_name, &curr_stat);
      strncpy(fileinfos[count].filename, entry->d_name, MAX_FILENAME_SIZE);
      fileinfos[count].filename[MAX_FILENAME_SIZE] = 0;
      fileinfos[count].size = curr_stat.st_size;

      count++;
    }
  }

  closedir(dir);
  return fileinfos;
}