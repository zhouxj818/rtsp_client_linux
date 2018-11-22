#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <limits.h>
#ifndef WIN32
/* According to POSIX.1-2001 */
#include <sys/select.h>
#include <unistd.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#else
//#include <windows.h>
#endif

#include "rtsp_type.h"
#include "rtsp_client.h"
#include "net.h"
#include "tpool.h"
#include "utils.h"
#include "rtp.h"
#include "rtcp.h"

int rtsp_client_init()
{
	int ret = 0;
#ifdef WIN32
	/* Initializing windows socket library */
	{
		WORD wVersionRequested;
		WSADATA wsaData;


		wVersionRequested = MAKEWORD(1, 1);
		ret = WSAStartup(wVersionRequested, &wsaData);
		if (ret)
		{
			
		}
	}
#endif
	return ret;
}


RtspClientSession * startRtspClient(int channel, char * url, RtspHandle handle, void * puser)
{
	RtspClientSession *cses = InitRtspClientSession();
	if ((NULL == cses) || (False == ParseUrl(url, cses))) {
		fprintf(stderr, "Error : Invalid Url Address.\n");
		return NULL;
	}
	cses->sess.channel = channel;
	cses->sess.ec = handle.clientErrorCb;
	cses->sess.pc = handle.playStartCb;
	cses->sess.setupCb = handle.setupCb;
	cses->sess.playCb= handle.playCb;
	cses->sess.teardownCb = handle.teardownCb;
	cses->sess.puser = puser;

	cses->rtspid = RtspCreateThread(RtspEventLoop, (void *)cses);
	if (cses->rtspid < 0x00) {
		fprintf(stderr, "RtspCreateThread Error!\n");
		free(cses);
		return NULL;
	}

	return cses;
}

static uint32_t check_url_prefix(char *url)
{
    if (!strncmp(url, PROTOCOL_PREFIX, strlen(PROTOCOL_PREFIX)))
        return True;
    return False;
}

static uint32_t parse_url_ip(char *url, char *ip)
{
    uint32_t offset = sizeof(PROTOCOL_PREFIX) - 1;
    char *pos = (char *)strchr((const char *)(url+offset), ':');
    if (NULL == pos){
        pos = (char *)strchr((const char *)(url+offset), '/');
        if (NULL == pos)
            return False;
    }
    uint32_t len  = (pos-url)-offset;
    strncpy(ip, (const char *)(url+offset), len);
    ip[len] = '\0';
    return True;
}
#define RTSP_PORT_MAX 100000
static uint32_t parse_url_port(char *url, uint32_t *port)
{
    uint32_t offset = sizeof(PROTOCOL_PREFIX) - 1;
    uint32_t len  = 0x00;
    char buf[8] = {0x00};
    char *pos = (char *)strchr((const char *)(url+offset), ':');
    if (NULL != pos){
        len = pos-url+1;
        pos = (char *)strchr((const char *)(url+len), '/');
        if (NULL == pos)
            return False;

        uint32_t size = (pos-url)-len;
        if (size > sizeof(buf)){
            fprintf(stderr, "Error: Invalid port\n");
            return False;
        }
        pos = url+len;
        strncpy((char *)buf, (const char *)pos, size);
        *port = atol((const char *)buf);
        if (*port > RTSP_PORT_MAX)
            return False;
    }

    return True;
}


uint32_t ParseUrl(char *url, RtspClientSession *cses)
{
    RtspSession *sess = &cses->sess;
    if (False == check_url_prefix(url))
        return False;
    /*if (False == parse_url_ip(url, sess->ip))
        return False;
    if (False == parse_url_port(url, &sess->port))
        return False;
        */
    if(False == ParseRtspUrl(url, sess))
		return False;
    strncpy((char *)sess->url, (const char *)url, \
            strlen((const char *)url) > sizeof(sess->url) ? \
            sizeof(sess->url) : strlen((const char *)url));
#ifdef RTSP_DEBUG
    printf("Host|Port|url : %s:%d:%s\n", sess->ip, sess->port, sess->url);
#endif

    return True;
}
#define TCP_BUF_SIZE 1500

void* RtspHandleTcpConnect(void* args)
{
    RtspClientSession *csess = (RtspClientSession *)(args);
    RtspSession *sess = &csess->sess;

    int32_t sockfd = sess->sockfd;
    int32_t num = 0x00, size = TCP_BUF_SIZE;
    int32_t rtpch = sess->transport.tcp.start;
    int32_t rtcpch = sess->transport.tcp.end;
    char    buf[TCP_BUF_SIZE], *pos = buf, framebuf[1920*1080];
    uint32_t length, framelen = 0x00;
    RtpOverTcp rot;
#ifdef SAVE_FILE_DEBUG
    FILE    *fp = fopen("1.264", "w+");
    if (NULL == fp){
        fprintf(stderr, "fopen error!\n");
        return NULL;
    }
#endif

    memset(framebuf, 0x00, sizeof(framebuf));
    do{
        pos = buf;
        num = TcpReceiveData(sockfd, (char *)&rot, sizeof(RtpOverTcp));
        if (num <= 0x00){
            fprintf(stderr, "recv error or connection closed!\n");
            break;
        }

        if (RTP_TCP_MAGIC == rot.magic){
            rot.len[1] &= 0xFF;
            length = GET_16(&rot.len[0]);
            int32_t size = length;
            do{
                num = TcpReceiveData(sockfd, pos, size);
                if (num <= 0x00){
                    fprintf(stderr, "recv error or connection closed!\n");
                    break;
                }
                size -= num;
                pos  += num;
            }while(size > 0x00);
            if (rtcpch == rot.ch){
                /* RTCP Protocl */
            }else if (rtpch == rot.ch){
                /* RTP Protocl */
                ParseRtp(buf, num, (RtpSession*)sess->rtpsess);
                length = sizeof(RtpHeader);
                num = UnpackRtpNAL(buf+length, num-length, framebuf+framelen, framelen);
                framelen += num;
                if (True == CheckRtpHeaderMarker(buf, length))
                {
#ifdef SAVE_FILE_DEBUG
                    fwrite(framebuf, framelen, 1, fp);
                    fflush(fp);
#endif
                    exit(0);
                    framelen = 0x00;
                }
            }
        }else{/* Check Rtsp Command */

        }
    }while(1);

#ifdef SAVE_FILE_DEBUG
    fclose(fp);
#endif
    printf("RtspHandleTcpConnect Quit!\n");
    return NULL;
}
#define  UDP_BUF_SIZE 4096
#define PRTSPSS_RTPSESSION ((RtpSession *)sess->rtpsess)
#define RTSPSS_RTPSESSION ((RtpSession *)sess->rtpsess)

void* RtspHandleUdpConnect(void* args)
{
    RtspClientSession *csess = (RtspClientSession *)(args);
    RtspSession *sess = &csess->sess;

    int32_t rtpfd = CreateUdpServer(sess->ip, sess->transport.udp.cport_from);
    int32_t rtcpfd = CreateUdpServer(sess->ip, sess->transport.udp.cport_to);
	SetSocketRcvBUf(rtpfd, 1024*1024*10);
    UdpConnect(&((RtpSession *)sess->rtpsess)->addrfrom, sess->ip, sess->transport.udp.sport_from, rtpfd);
    UdpConnect(&((RtpSession *)sess->rtpsess)->addrto, sess->ip, sess->transport.udp.sport_to, rtcpfd);
    int32_t num = 0x00, size = UDP_BUF_SIZE;
    char    buf[UDP_BUF_SIZE], framebuf[1920*1080];
    uint32_t length, framelen = 0x00;
#ifdef RTSP_DEBUG
    printf("------- server port: %d, %d ---------\n", \
            sess->transport.udp.sport_from, sess->transport.udp.sport_to);
    printf("rtp fd : %d, %d\n", rtpfd, rtcpfd);
    printf("ip, port : %s, %d\n", sess->ip, sess->transport.udp.cport_from);
#endif
#ifdef SAVE_FILE_DEBUG
	char file[32];
	sprintf(file, "%d.264", sess->channel);
    FILE    *fp = fopen(file, "w+");
    if (NULL == fp){
        fprintf(stderr, "fopen error!\n");
        return NULL;
    }
#endif

    gettimeofday(&((RtpSession *)sess->rtpsess)->rtcptv, NULL);
    struct timeval timeout;
    timeout.tv_sec=1;
    timeout.tv_usec=0;
    fd_set readfd;
    fd_set writefd;
    do{
        FD_ZERO(&readfd);
        FD_ZERO(&writefd);
        FD_SET(rtpfd, &readfd);
        FD_SET(rtcpfd, &readfd);
        FD_SET(rtcpfd, &writefd);
        FD_SET(rtpfd, &writefd);
        int32_t ret = select(rtcpfd+1, &readfd, &writefd, NULL, &timeout);
        if (-1 == ret){
            fprintf(stderr, "select error!\n");
            break;
        }
        if (1){
            static int times = 0x00;
            /*char tmp[] = {0xce, 0xfa, 0xed, 0xfe};*/
            char tmp[] = {0xce, 0xfa};
            if (times < 2){
                UdpSendData(rtpfd, tmp, sizeof(tmp), &((RtpSession *)sess->rtpsess)->addrfrom);
                times++;
            }

        }
        if (FD_ISSET(rtpfd, &readfd)){
            num = UdpReceiveData(rtpfd, buf, size, &((RtpSession *)sess->rtpsess)->addrfrom);
            if (num < 0x00){
                fprintf(stderr, "recv error or connection closed!\n");
                break;
            }
            ParseRtp(buf, num, (RtpSession *)sess->rtpsess);
            length = sizeof(RtpHeader);
            num = UnpackRtpNAL(buf+length, num-length, framebuf+framelen, framelen);
            framelen += num;
            if (True == CheckRtpHeaderMarker(buf, length))
            {
#ifdef SAVE_FILE_DEBUG
                fwrite(framebuf, framelen, 1, fp);
                fflush(fp);
#endif
				if(sess->playCb != NULL)
					sess->playCb(sess->channel, "video/H264", framebuf, framelen, NULL);
                framelen = 0x00;
            }
        }
        if (FD_ISSET(rtcpfd, &readfd)){
            num = UdpReceiveData(rtcpfd, buf, size, &((RtpSession *)sess->rtpsess)->addrto);
            if (num < 0x00){
                fprintf(stderr, "recv error or connection closed!\n");
                break;
            }
            uint32_t ret = ParseRtcp(buf, num, &((RtpSession *)sess->rtpsess)->stats);
            if (RTCP_BYE == ret){
                printf("Receive RTCP BYE!\n");
                break;
            }else if (RTCP_SR == ret){
                printf("Receive RTCP Sender Report!\n");
                gettimeofday(&((RtpSession *)sess->rtpsess)->rtcptv, NULL);
            }
        }
        if (FD_ISSET(rtcpfd, &writefd)){
            struct timeval now;
            gettimeofday(&now, NULL);
            if (0x02 < now.tv_sec - RTSPSS_RTPSESSION->rtcptv.tv_sec){
				RTSPSS_RTPSESSION->rtcptv = now;
                char tmp[512];
				uint32_t len = RtcpReceiveReport(tmp, sizeof(tmp), ((RtpSession *)sess->rtpsess));
                UdpSendData(rtcpfd, tmp, len, &((RtpSession *)sess->rtpsess)->addrto);
            }
        }
    }while(1);

#ifdef SAVE_FILE_DEBUG
    fclose(fp);
#endif
    close(rtpfd);
    close(rtcpfd);
    sess->state = RTSP_TEARDOWN;
    printf("RtspHandleUdpConnect Quit!\n");
    return NULL;
}

void* RtspEventLoop(void* args)
{
    RtspClientSession *csess = (RtspClientSession *)(args);
    RtspSession *sess = &csess->sess;
    int32_t fd = TcpConnect(sess->ip, sess->port);
    if (fd <= 0x00){
        fprintf(stderr, "Error: RtspConnect.\n");
        return NULL;
    }

    sess->sockfd = fd;
    do{
        if (True == isRtspClientSessionQuit(csess)){
            sess->state = RTSP_TEARDOWN;
        }else if ((False == RtspStatusMachine(sess)) || \
                (RTSP_QUIT == sess->state)){
            break;
        }
#if 0
        if (RTSP_KEEPALIVE == sess->state){
            if (RTP_AVP_UDP == sess->trans){
				if (sess->pc) {
					if(sess->state)
					sess->pc(sess->puser);
				}
				if (!sess->rtpid) {
					sess->rtpid = RtspCreateThread(RtspHandleUdpConnect, (void *)sess);
					if (sess->rtpid <= 0x00) {
						fprintf(stderr, "RtspCreateThread Error!\n");
						break;
					}
				}
				else {
					struct timeval timeout;
					timeout.tv_sec = 5;
					timeout.tv_usec = 0;
					fd_set readfd;

					FD_ZERO(&readfd);

					FD_SET(fd, &readfd);

					int32_t ret = select(fd + 1, &readfd, NULL, NULL, &timeout);
					if (ret < 0)
						break;
				}
            }else if (RTP_AVP_TCP == sess->trans){
                RtspHandleTcpConnect((void *)sess);
            }
        }
#endif
    }while(1);

    return NULL;
}

int32_t isRtspClientSessionQuit(RtspClientSession *rcsess)
{
    if (0x01 == rcsess->quit)
        return True;
    return False;
}

void SetRtspClientSessionQuit(RtspClientSession *rcsess)
{
    rcsess->quit = 0x01;
}

RtspClientSession* InitRtspClientSession()
{
    RtspClientSession *cses = (RtspClientSession *)calloc(1, sizeof(RtspClientSession));

    if (NULL == cses)
        return NULL;

    cses->quit = 0x00;
    RtspSession *sess = &cses->sess;
    sess->trans  = RTP_AVP_UDP;
    sess->state = RTSP_OPTIONS;
    sess->transport.tcp.start = 0x00;
    sess->transport.tcp.end = 0x01;
    memset((void *)&sess->buffctrl, 0x00 ,sizeof(sess->buffctrl));
	sess->rtpsess = calloc(1, sizeof(RtpSession));
    return cses;
}

void closeRtspClient(RtspClientSession *cses)
{
    if (NULL == cses)
        return;

	if (cses->rtspid) {
		TrykillThread(cses->rtspid);
	}

    RtspSession *sess = &cses->sess;
    CloseScokfd(sess->sockfd);
	free(sess->rtpsess);
    free(sess);
    sess = NULL;
    return;
}


int32_t ParseRtspUrl(char *url, RtspSession *sess)
{
	do {
        // Parse the URL as "rtsp://[<username>[:<password>]@]<server-address-or-name>[:<port>][/<stream-name>]"
        uint32_t offset = strlen(PROTOCOL_PREFIX);
        uint32_t i;
        char *from = url+offset;
        char *colonPasswordStart = NULL;
        char *p = NULL;

        for (p = from; *p != '\0' && *p != '/'; ++p)
        {
            if (*p == ':' && colonPasswordStart == NULL){
                colonPasswordStart = p;
            }else if (*p == '@'){
                if (colonPasswordStart == NULL) colonPasswordStart = p;
                char *usernameStart = from;
                uint32_t usernameLen = colonPasswordStart - usernameStart;

                for (i = 0; i < usernameLen; ++i)
                    sess->username[i] = usernameStart[i];
                sess->username[usernameLen] = '\0';

                char *passwordStart = colonPasswordStart;
                if (passwordStart < p) ++passwordStart; // skip over the ':'

                uint32_t passwordLen = p - passwordStart;
                uint32_t j = 0x00;
                for (; j < passwordLen; ++j)
                    sess->password[j] = passwordStart[j];
                sess->password[passwordLen] = '\0';
                from = p + 1; // skip over the '@'
                break;
            }
        }
        printf("sess username : %s\n", sess->username);
        printf("sess pssword : %s\n", sess->password);

        // Next, parse <server-address-or-name>
        char *to = sess->ip;
        for (i = 0; i < sizeof(sess->ip)+1; ++i)
        {
            if (*from == '\0' || *from == ':' || *from == '/')
            {
                *to = '\0';
                break;
            }
            *to++ = *from++;
        }
        if (i == sizeof(sess->ip)+1) {
            fprintf(stderr, "URL is too long");
            break;
        }

        printf("sess ip : %s\n", sess->ip);
        char nextChar = *from++;
        char tmp[8];
        to = tmp;
        if (nextChar == ':') {
            for (i = 0; i < sizeof(tmp); ++i)
            {
                if (*from == '/')
                {
                    *to = '\0';
                    break;
                }
                *to++ = *from++;
            }
            sess->port = atol((const char *)tmp);
        }
        printf("sess port : %d\n", sess->port);
    }while(0);

    return True;
}

