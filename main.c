#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#include<libssh2.h>
#include<libssh2_sftp.h>

#include "util.h"

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

int main(int argc, char *argv[])
{
    // --------------- debug
    unsigned char in[16] = {97, 98, 99, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0};
    int in_len = strlen(in);

    printf("--------------\n");
    print_buf(in, in_len);

    // --------------- vars
    const char *username = "root";
    const char *password = "LaBnCdAA";

    unsigned long hostaddr;
    int rc, sock, i, auth_pw = 0;
    struct sockaddr_in_sin;
    const char *fingerprint;
    char * userauthlist;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;

    // --------------- open socket to server
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
        ERR_EXIT("socket error");       
    }

    struct sockaddr_in servaddr;    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;  
    servaddr.sin_port = htons(22);
    servaddr.sin_addr.s_addr = inet_addr("47.52.27.162");

    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("connect error"); 

    // --------------- open ssh session
    rc = libssh2_init(0);
    if(rc != 0) {
        fprintf(stderr, "libssh2 initialization failed (%d)\n", rc);
        return -1;
    }
 
    session = libssh2_session_init();
    if(libssh2_session_handshake(session, sock)) {
        fprintf(stderr, "Failure establishing SSH session");
        return -1;
    }
    printf("--------------6\n");

    // --------------- login
    if (libssh2_userauth_password(session, username, password)) {
        fprintf(stderr, "Authentication by password failed!\n");
        printf("--------------9\n");
        goto shutdown;
    } else {
        fprintf(stderr, "Authentication by password succeeded.\n");
    }

    // --------------- create channel
    if(!(channel = libssh2_channel_direct_tcpip_ex(session, "120.27.157.103", 10001, "127.0.0.1", 36821))) {
        fprintf(stderr, "Failure creating channel");
        return -1;
        goto shutdown;
    }

    // --------------- send data
    libssh2_channel_write(channel, in, in_len);

    // --------------- close channel
    libssh2_channel_close(channel);
    libssh2_channel_free(channel);

    shutdown:
    libssh2_session_disconnect(session, "Normal Shutdown");
    libssh2_session_free(session);

    libssh2_exit();
    close(sock);

    printf("--------------10\n");
    return 0;
}
