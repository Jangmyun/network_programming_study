#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

void error_handling(char *message);

int test_inet_addr(){
  char *addr1 = "1.2.3.4";
  char *addr2 = "1.2.3.256";

  /*
    in_addr_t inet_addr(const char *string);

    return big-endian 32bit uint(success) / INADDR_NONE (-1) (error)
  */
  unsigned long conv_addr = inet_addr(addr1);
  if(conv_addr == INADDR_NONE){
    printf("Error\n");
  } else {
    printf("Network ordered integer addr: %#lx \n\n", conv_addr);
  }

  conv_addr = inet_addr(addr2);
  if(conv_addr == INADDR_NONE){
    printf("Error\n");
  } else {
    printf("Network ordered integer addr: %#lx \n\n", conv_addr);
  }

  return 0;
}

int test_inet_aton(){
  char *addr1 = "127.232.124.79";
  struct sockaddr_in addr_inet;

  /*
    int inet_aton(const char *string, struct in_addr *addr);

    return 1(success) / 0(error)
  */
  if(!inet_aton(addr1, &addr_inet.sin_addr)){
    error_handling("Conversion error");
  }else {
    printf("Network ordered integer addr: %#x \n\n", addr_inet.sin_addr.s_addr);
  }
   
  return 0;
}

int test_inet_ntoa(){
  struct sockaddr_in addr1, addr2;
  char *str_ptr;
  char str_arr[20];

  addr1.sin_addr.s_addr = htonl(0x1020304);
  addr2.sin_addr.s_addr = htonl(0x01010101);

  /*
    char* inet_ntoa(struct in_addr adr);
  */
  str_ptr = inet_ntoa(addr1.sin_addr);
  strcpy(str_arr, str_ptr);
  printf("Dotted-Decimal notation1: %s\n", str_ptr);

  inet_ntoa(addr2.sin_addr);
  printf("Dotted-Decimal notation1: %s\n", str_ptr);
  // 함수 내부적으로 메모리 공간을 할당해서 conversion된 문자열 정보를 저장

  printf("Dotted-Decimal notation1: %s\n", str_arr);

  return 0;
}


int main(int argc, char *argv[]) {
  test_inet_addr();

  test_inet_aton();

  test_inet_ntoa();

  return 0;
}

void error_handling(char *message){
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}