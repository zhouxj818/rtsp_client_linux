#ifndef _NET_H_
#define _NET_H_

#include <stdint.h>
#ifndef WIN32
#include <netinet/in.h>
#endif
#include "rtsp_type.h"

int32_t UdpConnect(struct sockaddr_in *addr, char *ip, uint32_t port, uint32_t sockfd);
int32_t TcpConnect(char *ip, uint32_t port);
int32_t SocketCork(int32_t fd, int32_t state);
int32_t SetSocketNonblock(int32_t sockfd);
int32_t SetSocketRcvBUf(int32_t sockfd, int bufSize);
int32_t TcpSendData(int32_t fd, char *buf, uint32_t size);
int32_t TcpReceiveData(int32_t fd, char *buf, uint32_t size);
void    CloseScokfd(int32_t sockfd);
int32_t CreateUdpServer(char *ip, uint32_t port);
int32_t UdpSendData(int32_t fd, char *buf, uint32_t size, struct sockaddr_in *remote);
int32_t UdpReceiveData(int32_t fd, char *buf, uint32_t size, struct sockaddr_in *addr);
int32_t GetSomePort ();

#endif
