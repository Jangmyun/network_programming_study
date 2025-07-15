#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 512
// #define DEBUG

void error_handling(char *message);

int main() {
	int server_socket = 0;
	int client_socket = 0;

	DIR* directory = NULL;
	struct dirent* entry = NULL;
	struct stat st;

	char filenameBuffer[BUFFER_SIZE];
	int currBuffSize = 0;

	directory = opendir(".");
	if(directory == NULL){
		perror("opendir() failed");
		exit(1);
	}
	
	
}
