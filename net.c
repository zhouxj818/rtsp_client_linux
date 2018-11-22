#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <fcntl.h>
#ifndef WIN32
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#endif
#include "port.h"
#include "net.h"


#define PORT_BASE 		(31688)


int32_t TcpConnect(char *ip, uint32_t port)
{
    int32_t res;
    int32_t sock_fd;
    struct sockaddr_in *remote;

    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd <= 0) {
	    printf("Error: could not create socket\n");
	    return -1;
    }

    remote = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    remote->sin_family = AF_INET;
    res = inet_pton(AF_INET, (const char*)ip, (void *) (&(remote->sin_addr.s_addr)));

    if (res < 0) {
	    fprintf(stderr, "Error: Can't set remote->sin_addr.s_addr\n");
	    free(remote);
	    return -1;
    }
    else if (res == 0) {
	    fprintf(stderr, "Error: Invalid address '%s'\n", ip);
	    free(remote);
	    return -1;
    }

    remote->sin_port = htons(port);
    if (connect(sock_fd,
                (struct sockaddr *) remote, sizeof(struct sockaddr)) == -1) {
        close(sock_fd);
        fprintf(stderr, "Error connecting to %s:%d\n", ip, port);
        free(remote);
        return -1;
    }

    free(remote);
    return sock_fd;
}

int32_t CreateUdpServer(char *ip, uint32_t port)
{
    int32_t sockfd;
    struct sockaddr_in addr;
    if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){
        perror ("socket");
        return -1;
    }
    memset(&addr, 0x00, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=htonl(INADDR_ANY) ;
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))<0){
        perror("connect");
        return -1;
    }

    return sockfd;
}

/* Connect to a UDP socket server and returns the file descriptor */
int32_t UdpConnect(struct sockaddr_in *addr, char *ip, uint32_t port, uint32_t sockfd)
{
    int32_t res;
    struct sockaddr_in *remote = addr;

    remote->sin_family = AF_INET;
    res = inet_pton(AF_INET, (const char*)ip, (void *) (&(remote->sin_addr.s_addr)));
    if (res < 0) {
	    fprintf(stderr, "Error: Can't set remote->sin_addr.s_addr\n");
	    return -1;
    }
    else if (res == 0) {
	    fprintf(stderr, "Error: Invalid address '%s'\n", ip);
	    return -1;
    }

    remote->sin_port = htons(port);
#if 0
    if (connect(sockfd,
                (struct sockaddr *)remote, sizeof(struct sockaddr)) == -1) {
        close(sockfd);
        fprintf(stderr, "Error connecting to %s:%d\n", ip, port);
        return -1;
    }
#endif

    return True;
}

int32_t SetSocketNonblock(int32_t sockfd)
{
#ifdef WIN32
	int ul = 1;
	ioctlsocket(sockfd, FIONBIO, &ul);
#else
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }
#endif
   return 0;
}

int32_t SetSocketRcvBUf(int32_t sockfd, int bufSize)
{
#ifdef WIN32
	///int ul = 1;
	///ioctlsocket(sockfd, FIONBIO, &ul);
#else	
	socklen_t len = sizeof(bufSize);
	if(setsockopt( sockfd, SOL_SOCKET, SO_RCVBUF, (int*)&bufSize, len ) < 0)
	{
        perror("setsockopt SO_RCVBUF faild");
        return -1;
	}
#endif
   return 0;
}


int32_t SocketCork(int32_t fd, int32_t state)
{
#ifdef WIN32
	int flag = 1;
	return setsockopt(fd,            /* socket affected */
		IPPROTO_TCP,     /* set option at TCP level */
		TCP_NODELAY,     /* name of option */
		(char *)&flag,  /* the cast is historical
						cruft */
		sizeof(int));    /* length of option value */
#else
    return setsockopt(fd, IPPROTO_TCP, TCP_CORK, &state, sizeof(state));
#endif
}

int32_t TcpSendData(int32_t fd, char *buf, uint32_t size)
{
    int32_t num = 0x00;
    num = send(fd, buf, size, 0);
#ifdef _DEBUG
	printf("\n-->:%s", buf);
#endif

    return num;
}


int32_t TcpReceiveData(int32_t fd, char *buf, uint32_t size)
{
    int32_t num = recv(fd, buf, size, 0);
	if (num > 0) {
#ifdef _DEBUG
		printf("\n<---:%s", buf);
#endif
	}
    return num;
}


void CloseScokfd(int32_t sockfd)
{
    close(sockfd);
    return;
}


int32_t UdpReceiveData(int32_t fd, char *buf, uint32_t size, struct sockaddr_in *addr)
{
    int32_t num = 0x00;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    num = recvfrom(fd, buf, size, 0, (struct sockaddr *)addr, &addr_len);

    return num;
}

int32_t UdpSendData(int32_t fd, char *buf, uint32_t size, struct sockaddr_in *remote)
{
    int32_t num = 0x00;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    num = sendto(fd, buf, size, 0, (struct sockaddr *)remote, addr_len);

    return num;
}


int32_t GetSomePort ()
{	
	static int32_t portTmp = 0;
	int32_t candidate = PORT_BASE + (portTmp + 2) % PORT_BASE; // so use ports 15000-30000
	int32_t tmp_socket;
	char sucess = 0;

	struct sockaddr_in server;
	while (!sucess) {
		memset(&server,0,sizeof(server));
		inet_pton(AF_INET, "0.0.0.0", &server.sin_addr);
		//memcpy(&(server.sin_addr),hp->h_addr,hp->h_length);
		server.sin_family = AF_INET;
		server.sin_port = htons(candidate);

		if ((tmp_socket = socket(AF_INET, SOCK_STREAM , 0)) < 0) 
			candidate+=2;
		else {
			if (bind (tmp_socket, (struct sockaddr *)&server, sizeof(server)) < 0) 
				candidate+=2;
			else 
				sucess = 1;
#ifdef WIN32
			closesocket (tmp_socket);
#else
			close (tmp_socket);
#endif
		}
	}

	portTmp = candidate;
	return candidate;
}

