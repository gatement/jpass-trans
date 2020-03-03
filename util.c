#include <stdio.h>
#include "util.h"

void print_buf(unsigned char * buffer, unsigned int len) {
  int i;
  for(i = 0; i < len; i++)
  {
    if (i % 16 == 0) printf("0x%04x: ", i);
    printf("%02x", buffer[i]);
    if (i % 2 == 1) printf(" ");
    if ((i+1) % 16 == 0 && (i + 1) < len) printf("\n");
  }
  printf("\n");
}

