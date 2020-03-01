#include<stdio.h>
#include <string.h>
#include "util.h"

int main(int argc, char *argv[])
{
    unsigned char in[16] = {97, 64, 58, 45, 78, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0};
    int in_len = strlen(in);

    printf("--------------\n");
    print_buf(in, in_len);

    return 0;
}
