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

#include "util.h"

#define MAX_CONN 2047
#define LISTEN_PORT 8117
#define JPASS_SERVER "192.168.56.1"
#define JPASS_PORT 8119

// include the listen fd
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

int create_listen_socket();

int main(int argc, char *argv[])
{
    // init listen socket
    int listenfd = create_listen_socket();

    struct sockaddr_in peeraddr; 
    struct sockaddr_in dstaddr;
    socklen_t peeraddrlen = sizeof(peeraddr); 
    socklen_t dstaddrlen = sizeof(dstaddr);

    int conn; 
    int conncount = 0;
    int nready;
    int clientcount;
    int i;

    struct pollfd clientspoll[MAX_CONN];
    struct pollfd clients[MAX_CONN];
    for (i = 0; i < MAX_CONN; i++) {
        clients[i].fd = -1;
    }
    clients[0].fd = listenfd;
    clients[0].events = POLLIN;

    char recvbuf[1024] = {0};

    // -- main loop ------------------------------------------------------------
    while (1)
    {
        // set clientspoll
        clientcount = 0;
        for (i = 0; i < MAX_CONN; i++) {
            if (clients[i].fd != -1) {
		clientspoll[clientcount++] = clients[i];
            }
        }

        // poll
        nready = poll(clientspoll, clientcount + 1, -1);
        if (nready == -1) {
            if (errno == EINTR) {
                continue;
	    }
            ERR_EXIT("poll error");
        } else if (nready == 0) {
            continue;
	}

        // -- start accept conn ------------------------------------------------
        if (clientspoll[0].revents & POLLIN) {
            // accept conn
            conn = accept(listenfd, (struct sockaddr *)&peeraddr, &peeraddrlen); 
            if (conn == -1) {
                ERR_EXIT("accept conn error");
	    }

            // save conn
            for (i = 1; i < MAX_CONN; i += 2) {
                if (clients[i].fd < 0) {
                    clients[i].fd = conn;
		    clients[i].events = POLLIN;
                    
                    conncount += 1;
                    break;
                }
            }

            if ((i+1) >= MAX_CONN) {
                fprintf(stderr, "too many clients, max: %d\n", MAX_CONN);
                exit(EXIT_FAILURE);
            }

            // get dst addr
	    if (getsockname(conn, (struct sockaddr *)&dstaddr, &dstaddrlen) < 0) {
		ERR_EXIT("get sock name error");
            }

            // print log
            printf("[%d]: recv conn %s:%d -> ", conncount, inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
            printf("%s:%d\n", inet_ntoa(dstaddr.sin_addr), ntohs(dstaddr.sin_port));

	    // create jpass server conn
            int jpassfd = create_jpass_socket();
	    clients[i+1].fd = jpassfd;
	    clients[i+1].events = POLLIN;

	    // send jpass sock header data (big-endian ip + big-endian port)
            unsigned int addr = inet_addr(inet_ntoa(dstaddr.sin_addr));
            recvbuf[0] = (unsigned char)(addr >> 24);
            recvbuf[1] = (unsigned char)(addr >> 16);
            recvbuf[2] = (unsigned char)(addr >> 8);
            recvbuf[3] = (unsigned char)(addr);
            recvbuf[4] = (unsigned char)(dstaddr.sin_port >> 8);
            recvbuf[5] = (unsigned char)(dstaddr.sin_port);
            write(jpassfd, recvbuf, 6);

            if (--nready <= 0) continue;
        }

        // -- read data from conn ----------------------------------------------
        for (i = 1; i <= clientcount; i++) {
            conn = clientspoll[i].fd;
            if (conn == -1) continue;

            // start receiving data
            if (clientspoll[i].revents & POLLIN) {
                memset(recvbuf, 0, sizeof(recvbuf));
		int ret = read(conn, recvbuf, sizeof(recvbuf));
                if (ret == -1) {
                    ERR_EXIT("read conn error");
                } else if (ret == 0) {
                    if (i % 2 == 1) {
                        // client conn
			fprintf(stderr, "client conn close.\n");
                        close(conn);
                        clients[i].fd = -1;

                        // close the server conn as well
                        close(clients[i+1].fd);
                        clients[i+1].fd = -1;
                    } else {
                        // server conn
			fprintf(stderr, "server conn close.\n");
                        close(conn);
                        clients[i].fd = -1;

                        // close the client conn as well
                        close(clients[i-1].fd);
                        clients[i-1].fd = -1;
		    }

                    conncount --;
                } else {
                    // debug
                    //print_buf(recvbuf, ret);
                    fputs(recvbuf, stdout);

		    // write other part
                    if (i % 2 == 1) {
                        write(clients[i+1].fd, recvbuf, ret);
                    } else {
                        write(clients[i-1].fd, recvbuf, ret);
                    }
                }

                if (--nready <= 0) break;
            }
        }
    }

    close(listenfd);

    printf("application exited.\n");
    return 0;
}

// return listen fd
int create_listen_socket() {
    int listenfd;

    if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
        ERR_EXIT("create listening socket error");
    }

    struct sockaddr_in listenaddr;
    memset(&listenaddr, 0, sizeof(listenaddr));
    listenaddr.sin_family = AF_INET;
    listenaddr.sin_port = htons(LISTEN_PORT);
    listenaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(listenfd, SOL_IP, IP_TRANSPARENT, &on, sizeof(on)) < 0) {
        ERR_EXIT("setsockopt IP_TRANSPARENT error");
    }
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        ERR_EXIT("setsockopt SO_REUSEADDR error");
    }

    if (bind(listenfd, (struct sockaddr *)&listenaddr, sizeof(listenaddr)) < 0){
        ERR_EXIT("bind error");
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen error");
    }

    fprintf(stderr, "listening on port %d\n", LISTEN_PORT);

    return listenfd;
}

// return socket fd
int create_jpass_socket() {
    int jpassfd;

    if ((jpassfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
        ERR_EXIT("create jpass socket error");       
    }

    struct sockaddr_in sshservaddr;
    memset(&sshservaddr, 0, sizeof(sshservaddr));
    sshservaddr.sin_family = AF_INET;  
    sshservaddr.sin_port = htons(JPASS_PORT);
    sshservaddr.sin_addr.s_addr = inet_addr(JPASS_SERVER);

    if (connect(jpassfd, (struct sockaddr *)&sshservaddr, sizeof(sshservaddr)) < 0) {
        ERR_EXIT("connect jpass server error"); 
    }

    return jpassfd;
}
