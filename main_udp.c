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
#define JPASS_SERVER "192.168.56.1"
#define JPASS_PORT 8117

int udpListenPort = 0;
int serviceType = 0; // 0: udp, 1: dns

int create_udp_server_socker();
int create_jpass_client_tcp_socket();

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("ERROR: please input service type (udp|dns) and UDP listening port parm.\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "dns") == 0) {
        serviceType = 1;
        printf("starting DNS server.\n");
    } else {
        serviceType = 0;
        printf("starting UDP tunnel.\n");
    }
    udpListenPort = atoi(argv[2]);

    // init listening socket
    int udplistenfd = create_udp_server_socker();

    struct sockaddr_in dstaddr;
    socklen_t dstaddrlen = sizeof(dstaddr);

    int conn;
    int size;
    int conncount = 0;
    int nready;
    int clientcount;
    int i;

    struct sockaddr_in clientAddrs[MAX_CONN];

    struct pollfd clientspoll[MAX_CONN];
    struct pollfd clients[MAX_CONN];
    for (i = 0; i < MAX_CONN; i++) {
        clients[i].fd = -1;
    }
    clients[0].fd = udplistenfd;
    clients[0].events = POLLIN;

    unsigned char recvbuf[8192] = {0};

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
            printf("poll error\n");
        } else if (nready == 0) {
            continue;
	}

        // -- recv from UDP ----------------------------------------------------
        if (clientspoll[0].revents & POLLIN) {
            struct sockaddr_in peeraddr; 
            socklen_t peeraddrlen = sizeof(peeraddr); 
            memset(recvbuf, 0, sizeof(recvbuf));
            size = recvfrom(udplistenfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&peeraddr, &peeraddrlen);
            if (size == -1) {
                printf("recv from UDP error\n");
	    } else if (size != 0) {
                // get dst addr
                // TODO
		//if (getsockname(conn, (struct sockaddr *)&dstaddr, &dstaddrlen) < 0) {
		//    printf("get sock name error\n");
                //}

                // print log
                printf("[%d]: recv udp %s:%d -> ", conncount, inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
                printf("%s:%d\n", inet_ntoa(dstaddr.sin_addr), ntohs(dstaddr.sin_port));

		// create jpass server conn
                int jpassfd = create_jpass_client_tcp_socket();

                // save jpass conn
                for (i = 1; i < MAX_CONN; i++) {
                    if (clients[i].fd < 0) {
                        clients[i].fd = jpassfd;
			clients[i].events = POLLIN;
                        conncount ++;
                        break;
                    }
                }

                // save clientAddr
                clientAddrs[i] = peeraddr;

                if (i >= MAX_CONN) {
                    printf("too many clients, max: %d\n", MAX_CONN);
                    exit(EXIT_FAILURE);
                }

		// send jpass sock header data (big-endian ip + big-endian port)
                //unsigned int addr = htonl(inet_addr(inet_ntoa(dstaddr.sin_addr)));
                unsigned int addr = htonl(inet_addr("8.8.8.8"));
                //int port = htons(dstaddr.sin_port);
                int port = htons(53);
                recvbuf[0] = (unsigned char)(addr >> 24);
                recvbuf[1] = (unsigned char)(addr >> 16);
                recvbuf[2] = (unsigned char)(addr >> 8);
                recvbuf[3] = (unsigned char)(addr);
                recvbuf[4] = (unsigned char)(port >> 8);
                recvbuf[5] = (unsigned char)(port);
                write(jpassfd, recvbuf, 6);
            }
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
		    printf("read jpass conn error\n");
                } else if (ret == 0) {
		    //printf("jpass conn close.\n");
                    close(conn);
                    clients[i].fd = -1;
                    conncount --;
                } else {
                    // debug
                    //print_buf(recvbuf, ret);
                    //fputs(recvbuf, stdout);

		    // write other part
                    struct sockaddr_in peeraddr = clientAddrs[i]; 
                    socklen_t peeraddrlen = sizeof(peeraddr); 
                    sendto(udplistenfd, recvbuf, ret, 0, (struct sockaddr *)&peeraddr, peeraddrlen);
                }

                if (--nready <= 0) break;
            }
        }
    }

    close(udplistenfd);

    printf("application exited.\n");
    return 0;
}

// return socket fd
int create_udp_server_socker() {
    int udplistenfd;
    if ((udplistenfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("create udp listening socket error\n");
    }

    struct sockaddr_in listenaddr;
    memset(&listenaddr, 0, sizeof(listenaddr));
    listenaddr.sin_family = AF_INET;
    listenaddr.sin_port = htons(udpListenPort);
    listenaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    if (setsockopt(udplistenfd, SOL_IP, IP_TRANSPARENT, &on, sizeof(on)) < 0) {
        printf("udp listening socket setsockopt IP_TRANSPARENT error\n");
    }
    /*
    if (setsockopt(udplistenfd, IPPROTO_IP, IP_RECVORIGDSTADDR, &on, sizeof(on)) < 0) {
        printf("udp listening socket setsockopt IP_RECVORIGDSTADDR error\n");
    }
    */
    if (setsockopt(udplistenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        printf("udp listening socket setsockopt SO_REUSEADDR error\n");
    }

    if (bind(udplistenfd, (struct sockaddr *)&listenaddr, sizeof(listenaddr)) < 0) {
        printf("udp listen socket bind error\n");
    }

    printf("listening on UDP port %d\n", udpListenPort);

    return udplistenfd;
}

// return socket fd
int create_jpass_client_tcp_socket() {
    int jpassfd;

    if ((jpassfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
        printf("create jpass client tcp socket error\n");       
    }

    struct sockaddr_in sshservaddr;
    memset(&sshservaddr, 0, sizeof(sshservaddr));
    sshservaddr.sin_family = AF_INET;  
    sshservaddr.sin_port = htons(JPASS_PORT);
    sshservaddr.sin_addr.s_addr = inet_addr(JPASS_SERVER);

    //printf("connecting to jpass server: %s:%d\n", JPASS_SERVER, JPASS_PORT);

    if (connect(jpassfd, (struct sockaddr *)&sshservaddr, sizeof(sshservaddr)) < 0) {
        printf("connect jpass server error\n"); 
    }

    return jpassfd;
}

