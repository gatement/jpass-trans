#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<unistd.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<signal.h>
#include<poll.h>

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
    const int MAX_CONN = 2048;
    const char *USERNAME = "root";
    const char *PASSWORD = "LaBnCdAA";

    signal(SIGPIPE, SIG_IGN);

    // == init TCP socket ======================================================
    // -- create listening socket ----------------------------------------------
    int listenfd;
    if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
        ERR_EXIT("socket error");
    }

    // -- set socket options ---------------------------------------------------
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        ERR_EXIT("setsockopt SO_REUSEADDR error");
    }
    if (setsockopt(listenfd, SOL_IP, IP_TRANSPARENT, &on, sizeof(on)) < 0) {
        ERR_EXIT("setsockopt IP_TRANSPARENT error");
    }

    // -- bind and listen ------------------------------------------------------
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8119);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        ERR_EXIT("bind error");
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen error");
    }

    // -- init vars ------------------------------------------------------------
    struct sockaddr_in peeraddr; 
    socklen_t peerlen = sizeof(peeraddr); 

    int conn_count = 0;
    int conn; 
    int i;
    int nready;

    struct pollfd client[MAX_CONN];
    for (i = 0; i < MAX_CONN; i++) {
        client[i].fd = -1;
    }
    client[0].fd = listenfd;
    client[0].events = POLLIN;

    // == init ssh session =====================================================
    
    // == start accept connections =============================================
    /*
    while (1)
    {
        nready = poll(client, maxi + 1, -1);
        if (nready == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("poll error");
        }

        if (nready == 0)
            continue;

        if (client[0].revents & POLLIN)
        {

            conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peerlen); 
            if (conn == -1)
                ERR_EXIT("accept error");

            for (i = 1; i < 2048; i++)
            {
                if (client[i].fd < 0)
                {
                    client[i].fd = conn;
                    if (i > maxi)
                        maxi = i;
                    break;
                }
            }

            if (i == 2048)
            {
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }

            printf("%d: recv connect ip=%s port=%d\n", ++conn_count, inet_ntoa(peeraddr.sin_addr),
                   ntohs(peeraddr.sin_port));

            client[i].events = POLLIN;

            if (--nready <= 0)
                continue;
        }

        for (i = 1; i <= maxi; i++)
        {
            conn = client[i].fd;
            if (conn == -1)
                continue;
            if (client[i].revents & POLLIN)
            {

                char recvbuf[1024] = {0};
                int ret = readline(conn, recvbuf, 1024);
                if (ret == -1)
                    ERR_EXIT("readline error");
                else if (ret  == 0) 
                {
                    //printf("client  close \n");
                    client[i].fd = -1;
                    close(conn);
                }

                fputs(recvbuf, stdout);
                writen(conn, recvbuf, strlen(recvbuf));

                if (--nready <= 0)
                    break;
            }
        }

    }


    
*/


    /*
    // --------------- debug
    unsigned char in[16] = {97, 98, 99, 10, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0};
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
    if(!(channel = libssh2_channel_direct_tcpip(session, "120.27.157.103", 10001))) {
        fprintf(stderr, "Failure creating channel");
        return -1;
        goto shutdown;
    }

    // --------------- send data
    libssh2_channel_write(channel, in, in_len);
    sleep(200);
    libssh2_channel_write(channel, in, in_len);

    // --------------- close channel
    libssh2_channel_close(channel);
    libssh2_channel_free(channel);

    shutdown:
    libssh2_session_disconnect(session, "Normal Shutdown");
    libssh2_session_free(session);

    libssh2_exit();
    close(sock);





    char recvbuf[1024];
    while (1)
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        int ret = read(conn, recvbuf, sizeof(recvbuf));
        fputs(recvbuf, stdout);
        write(conn, recvbuf, ret);  
    }



    */
    printf("--------------10\n");
    return 0;
}
