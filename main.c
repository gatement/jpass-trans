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

#include "util.h"

#define MAX_CONN 2048
#define LISTEN_PORT 8117
#define SSH_SERVER "120.27.157.103"
#define SSH_PORT 9812
#define SSH_USERNAME "johnson"
#define SSH_PASSWORD "123456"

// include the listen fd
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

int conn; 

int create_listen_socket();
int create_ssh_socket();
void init_ssh_session(LIBSSH2_SESSION *session, int sshfd);
void handler(int sig);

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    // init listen socket
    int listenfd = create_listen_socket();

    // init ssh session
    int sshfd = create_ssh_socket();
    LIBSSH2_SESSION *session;
    init_ssh_session(session, sshfd);

    struct sockaddr_in peeraddr; 
    struct sockaddr_in dstaddr;
    socklen_t peeraddrlen = sizeof(peeraddr); 
    socklen_t dstaddrlen = sizeof(dstaddr);

    int conncount = 0;
    int nready;
    int i, clientcount;

    struct pollfd clientspoll[MAX_CONN];
    struct pollfd clients[MAX_CONN];
    for (i = 0; i < MAX_CONN; i++) {
        clients[i].fd = -1;
    }
    clients[0].fd = listenfd;
    clients[0].events = POLLIN;

    LIBSSH2_CHANNEL *channels[MAX_CONN-1];

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
            for (i = 1; i < MAX_CONN; i++) {
                if (clients[i].fd < 0) {
                    clients[i].fd = conn;
		    clients[i].events = POLLIN;
                    conncount++;
                    break;
                }
            }

            if (i == MAX_CONN) {
                fprintf(stderr, "too many clients, max: %d\n", MAX_CONN);
                exit(EXIT_FAILURE);
            }

            // get dst addr
	    if (getsockname(conn, (struct sockaddr *)&dstaddr, &dstaddrlen) < 0) {
		ERR_EXIT("get sock name error");
            }
            int dstport = ntohs(dstaddr.sin_port);
            const char *dstip = inet_ntoa(dstaddr.sin_addr);

            // print log
            printf("[%d]: recv conn %s:%d -> %s:%d\n", conncount, inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), dstip, dstport);

            // create ssh channel
	    LIBSSH2_CHANNEL *channel;
	    if(!(channel = libssh2_channel_direct_tcpip(session, dstip, dstport))) {
		ERR_EXIT("fail to create channel");
	    }
            channels[i-1] = channel;

            // fork to let channel receive data
            pid_t pid = fork();
	    if (pid == -1) {
		ERR_EXIT("fork error");
            } else if (pid == 0) { // child process
		signal(SIGUSR1, handler);

		close(listenfd);
		close(sshfd);

		char recvbuf[1024] = {0};
		while (1) {
                    int ret = libssh2_channel_read(hannel, recvbuf, sizeof(recvbuf));
                    if (ret < 0) {
			ERR_EXIT("read data from channel error");
                    } else if (ret > 0) {
			write(conn, recvbuf, ret);
                    }
                }
	    } else { // parent process
		close(conn);
            }

            if (--nready <= 0) continue;
        }

        // -- read data from conn ----------------------------------------------
        for (i = 1; i <= clientcount; i++) {
            conn = clientspoll[i].fd;
            if (conn == -1) continue;

            // start receiving data
            if (clientspoll[i].revents & POLLIN) {
		int ret = read(conn, recvbuf, sizeof(recvbuf));
                if (ret == -1) {
                    ERR_EXIT("readline error");
                } else if (ret == 0) {
                    //printf("conn close\n");
                    clients[i].fd = -1;
                    conncount--;
                    close(conn);
		    libssh2_channel_close(channels[i-1]);
		    libssh2_channel_free(channels[i-1]);
                } else {
                    // write to ssh channel
                    print_buf(recvbuf, ret);
		    libssh2_channel_write(channels[i-1], recvbuf, ret);
                }

                if (--nready <= 0) break;
            }
        }
    } 
    //sleep(10);

    libssh2_session_disconnect(session, "Normal Shutdown");
    libssh2_session_free(session);

    close(listenfd);
    close(sshfd);
    libssh2_exit();

    printf("application exited.\n");
    return 0;
}

// return listen fd
int create_listen_socket() {
    int listenfd;

    if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
        ERR_EXIT("create listening socket error");
    }

    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        ERR_EXIT("setsockopt SO_REUSEADDR error");
    }
    if (setsockopt(listenfd, SOL_IP, IP_TRANSPARENT, &on, sizeof(on)) < 0) {
        ERR_EXIT("setsockopt IP_TRANSPARENT error");
    }

    struct sockaddr_in listenaddr;
    memset(&listenaddr, 0, sizeof(listenaddr));
    listenaddr.sin_family = AF_INET;
    listenaddr.sin_port = htons(LISTEN_PORT);
    listenaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *)&listenaddr, sizeof(listenaddr)) < 0){
        ERR_EXIT("bind error");
    }

    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen error");
    }

    return listenfd;
}

// return ssh fd
int create_ssh_socket() {
    int sshfd;

    if ((sshfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)  {
        ERR_EXIT("create ssh socket error");       
    }

    struct sockaddr_in sshservaddr;
    memset(&sshservaddr, 0, sizeof(sshservaddr));
    sshservaddr.sin_family = AF_INET;  
    sshservaddr.sin_port = htons(SSH_PORT);
    sshservaddr.sin_addr.s_addr = inet_addr(SSH_SERVER);

    if (connect(sshfd, (struct sockaddr *)&sshservaddr, sizeof(sshservaddr)) < 0) {
        ERR_EXIT("connect ssh server error"); 
    }

    return sshfd;
}

void init_ssh_session(LIBSSH2_SESSION *session, int sshfd) {
    if((libssh2_init(0)) < 0) {
        ERR_EXIT("init libssh2 error");       
    }

    session = libssh2_session_init();
    if(libssh2_session_handshake(session, sshfd)) {
        ERR_EXIT("fail to do ssh session handshake");       
    }

    if (libssh2_userauth_password(session, SSH_USERNAME, SSH_PASSWORD)) {
        ERR_EXIT("fail to do ssh auth");       
    } else {
        fprintf(stderr, "ssh authenticated by password succeeded.\n");
    }
}

void handler(int sig) {
    printf("processor recv a sig=%d\n", sig);
    close(conn);
    exit(EXIT_SUCCESS);
}
