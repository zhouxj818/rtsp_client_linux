#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include "rtsp_type.h"
#include "rtp.h"
#include "rtcp.h"
#include "utils.h"
#include "rtsp.h"
#include "tpool.h"
#include "rtsp_common.h"
#include "rtsp_response.h"
#include "net.h"

static int32_t RtspSendKeepAliveCommand(RtspSession *sess);
static int32_t RtspSendOptionsCommand(RtspSession *sess)
{
    int32_t size = sizeof(sess->buffctrl.buffer);
    int32_t sock = sess->sockfd;
    char *buf = sess->buffctrl.buffer;

    memset(buf, '\0', size);
    int32_t num = snprintf(buf, size, CMD_OPTIONS, sess->ip, sess->port, sess->cseq);
    if (num < 0x00){
        fprintf(stderr, "%s : snprintf error!\n", __func__);
        return False;
    }

    num = TcpSendData(sock, buf, (uint32_t)num);
    if (num < 0){
        fprintf(stderr, "%s : Send Error\n", __func__);
        return False;
    }

    return True;
}

int32_t RtspOptionsCommand(RtspSession *sess)
{
    int32_t num;
    int32_t size = sizeof(sess->buffctrl.buffer);
    char *buf = sess->buffctrl.buffer;
    int32_t sock = sess->sockfd;
#ifdef RTSP_DEBUG
    printf("++++++++++++++++++  OPTIONS: command  +++++++++++++++++++++\n");
#endif
    if (False == RtspSendOptionsCommand(sess))
        return False;
#ifdef RTSP_DEBUG
    printf("OPTIONS Request: %s\n", buf);
#endif
    memset(buf, '\0', size);
    num = RtspReceiveResponse(sock, &sess->buffctrl);
    if (num <= 0) {
        printf("Error: Server did not respond properly, closing...");
        return False;
    }

#ifdef RTSP_DEBUG
    printf("\nOptions Reply: %s\n", buf);
#endif
    if (False == RtspCheckResponseStatus(buf))
        return False;


    ParseOptionsPublic(buf, num, sess);

	sess->state = RTSP_DESCRIBE;

    return True;
}

static int32_t RtspSendDescribeCommand(RtspSession *sess, char *buf, uint32_t size)
{
    int32_t sock = sess->sockfd;
    memset(buf, '\0', size);
    int32_t num = snprintf(buf, size, CMD_DESCRIBE, sess->url, sess->cseq);
    if (num < 0x00){
        fprintf(stderr, "%s : snprintf error!\n", __func__);
        return False;
    }

    num = TcpSendData(sock, buf, (uint32_t)num);
    if (num < 0){
        fprintf(stderr, "%s : Send Error\n", __func__);
        return False;
    }

    return True;
}

int32_t RtspDescribeCommand(RtspSession *sess)
{
    int32_t num;
    int32_t size = sizeof(sess->buffctrl.buffer);
    char *buf = sess->buffctrl.buffer;
    int32_t sock = sess->sockfd;

#ifdef RTSP_DEBUG
    printf("++++++++++++++++++++++  DESCRIBE: command  +++++++++++++++++++++++++++\n");
#endif

    if (False == RtspSendDescribeCommand(sess, buf, size))
        return False;

#ifdef RTSP_DEBUG
    printf("DESCRIBE Request: %s\n", buf);
#endif

    memset(buf, '\0', size);
    num = RtspReceiveResponse(sock, &sess->buffctrl);
    if (num <= 0) {
        printf("Error: Server did not respond properly, closing...");
        return False;
    }

#ifdef RTSP_DEBUG
    printf("\nDescribe Reply: %s\n", buf);
#endif
    if (False == RtspCheckResponseStatus(buf))
        return False;
    ParseSdpProto(buf, num, sess);

	strncpy(sess->sdp, sess->buffctrl.buffer, num);
	sess->sdp[num] = 0;

    sess->state = RTSP_SETUP;

    return True;
}

static int32_t RtspSendSetupCommand(RtspSession *sess)
{
    int32_t size = sizeof(sess->buffctrl.buffer);
    int32_t num = 0x00;
    int32_t sock = sess->sockfd;
    char *buf = sess->buffctrl.buffer;
    char url[256];

    memset(buf, '\0', size);
    memset(url, '\0', sizeof(url));
    if (NULL == strstr(sess->vmedia.control, PROTOCOL_PREFIX)){
        int32_t len = strlen(sess->url);
        strncpy(url, sess->url, len);
        url[len] = '/';
        url[len+1] = '\0';
    }
    strncat(url, sess->vmedia.control, strlen(sess->vmedia.control));
#ifdef RTSP_DEBUG
    printf("SETUP URL: %s\n", url);
#endif
    if (RTP_AVP_TCP == sess->trans){
        num = snprintf(buf, size, CMD_TCP_SETUP, url, sess->cseq);
    }else if (RTP_AVP_UDP == sess->trans){
    	int udpPort = GetSomePort();
        num = snprintf(buf, size, CMD_UDP_SETUP, url, sess->cseq, udpPort, udpPort+1);
    }
    if (num < 0x00){
        fprintf(stderr, "%s : snprintf error!\n", __func__);
        return False;
    }
    num = TcpSendData(sock, buf, (uint32_t)num);
    if (num < 0){
        fprintf(stderr, "%s : Send Error\n", __func__);
        return False;
    }
    return True;
}


static int32_t RtspSendSetupCommand2(RtspSession *sess)
{
	int32_t size = sizeof(sess->buffctrl.buffer);
	int32_t num = 0x00;
	int32_t sock = sess->sockfd;
	char *buf = sess->buffctrl.buffer;
	char url[256];

	memset(buf, '\0', size);
	memset(url, '\0', sizeof(url));
	if (NULL == strstr(sess->amedia.control, PROTOCOL_PREFIX)) {
		int32_t len = strlen(sess->url);
		strncpy(url, sess->url, len);
		if (sess->amedia.control[0]) {
			url[len] = '/';
			url[len + 1] = '\0';
		}
		else {
			url[len] = 0;
		}
	}
	strncat(url, sess->amedia.control, strlen(sess->amedia.control));
#ifdef RTSP_DEBUG
	printf("SETUP URL: %s\n", url);
#endif
	if (RTP_AVP_TCP == sess->trans2) {
		num = snprintf(buf, size, CMD_TCP_SETUP, url, sess->cseq);
	}
	else if (RTP_AVP_UDP == sess->trans2) {		
    	int udpPort = GetSomePort();
		if (sess->sessid[0]) {
			num = snprintf(buf, size, CMD_UDP_SETUP_S, url, sess->cseq, udpPort, udpPort+1, sess->sessid);
		}
		else {
			num = snprintf(buf, size, CMD_UDP_SETUP, url, sess->cseq, udpPort, udpPort+1);
		}
	}
	if (num < 0x00) {
		fprintf(stderr, "%s : snprintf error!\n", __func__);
		return False;
	}
	num = TcpSendData(sock, buf, (uint32_t)num);
	if (num < 0) {
		fprintf(stderr, "%s : Send Error\n", __func__);
		return False;
	}
	return True;
}


int32_t RtspSetupCommand(RtspSession *sess)
{
    int32_t num;
    int32_t size = sizeof(sess->buffctrl.buffer);
    char *buf = sess->buffctrl.buffer;
    int32_t sock = sess->sockfd;

#ifdef RTSP_DEBUG
    printf("++++++++++++++++++++  SETUP: command  +++++++++++++++++++++++++\n");
#endif

	if (sess->vmedia.control[0]) {
		if (False == RtspSendSetupCommand(sess))
			return False;

#ifdef RTSP_DEBUG
		printf("SETUP Request: %s\n", buf);
#endif
		memset(buf, '\0', size);
		num = RtspReceiveResponse(sock, &sess->buffctrl);
		if (num <= 0) {
			fprintf(stderr, "Error: Server did not respond properly, closing...");
			return False;
		}

#ifdef RTSP_DEBUG
		printf("SETUP Reply: %s\n", buf);
#endif
		if (False == RtspCheckResponseStatus(buf))
			return False;

		if (RTP_AVP_UDP == sess->trans) {
			ParseUdpPort(buf, num, sess);
		}
		else {
			ParseInterleaved(buf, num, sess);
		}
		ParseSessionID(buf, num, sess);
	}

	if (sess->amedia.control[0]) {
		if (sess->vmedia.control[0]) {
			sess->cseq++;
		}

		if (False == RtspSendSetupCommand2(sess))
			return False;
#ifdef RTSP_DEBUG
		printf("SETUP Request: %s\n", buf);
#endif
		memset(buf, '\0', size);
		num = RtspReceiveResponse(sock, &sess->buffctrl);
		if (num <= 0) {
			fprintf(stderr, "Error: Server did not respond properly, closing...");
			return False;
		}

#ifdef RTSP_DEBUG
		printf("SETUP Reply: %s\n", buf);
#endif
		if (False == RtspCheckResponseStatus(buf))
			return False;

		if (RTP_AVP_UDP == sess->trans) {
			ParseUdpPort2(buf, num, sess);
		}
		else {
			ParseInterleaved(buf, num, sess);
		}

		if (!sess->sessid[0]){
		ParseSessionID(buf, num, sess);
		}
	}

	if(sess->setupCb != NULL)
		sess->setupCb(sess->channel, 96);

    sess->packetization = 1;
    sess->state = RTSP_PLAY;
    return True;
}

static int32_t RtspSendPlayCommand(RtspSession *sess)
{
    int32_t size = sizeof(sess->buffctrl.buffer);
    int32_t sock = sess->sockfd;
    char *buf = sess->buffctrl.buffer;

    memset(buf, '\0', size);
    int32_t num = snprintf(buf, size, CMD_PLAY, sess->url, sess->cseq, sess->sessid);
    if (num < 0x00){
        fprintf(stderr, "%s : snprintf error!\n", __func__);
        return False;
    }
    num = TcpSendData(sock, buf, (uint32_t)num);
    if (num < 0){
        fprintf(stderr, "%s : Send Error\n", __func__);
        return False;
    }

    return True;
}

void* RtspHandleUdpConnect(void* args);
void* RtspHandleTcpConnect(void* args);

int32_t RtspPlayCommand(RtspSession *sess)
{
    int32_t num;
    int32_t size = sizeof(sess->buffctrl.buffer);
    int32_t sock = sess->sockfd;
    char *buf = sess->buffctrl.buffer;

#ifdef RTSP_DEBUG
    printf("+++++++++++++++++++  PLAY: command  ++++++++++++++++++++++++++\n");
#endif
    if (False == RtspSendPlayCommand(sess))
        return False;

#ifdef RTSP_DEBUG
    printf("PLAY Request: %s\n", buf);
#endif

    memset(buf, '\0', size);
    num = RtspReceiveResponse(sock, &sess->buffctrl);
    if (num <= 0) {
        fprintf(stderr, "Error: Server did not respond properly, closing...");
        return False;
    }

#ifdef RTSP_DEBUG
    printf("PLAY Reply: %s\n", buf);
#endif
    if (False == RtspCheckResponseStatus(buf))
        return False;
    ParseTimeout(buf, num, sess);
    gettimeofday(&sess->last_cmd_time, NULL);

	if (sess->pc) {
		sess->pc(sess->puser);
	}
	else
	{
		if (RTP_AVP_UDP == sess->trans) {
			if (sess->pc) {
				if (sess->state)
					sess->pc(sess->puser);
			}
			if (!sess->rtpid) {
				sess->rtpid = RtspCreateThread(RtspHandleUdpConnect, (void *)sess);
				if (sess->rtpid <= 0x00) {
					fprintf(stderr, "RtspCreateThread Error!\n");
					return -1;
				}
			}
			else {
				struct timeval timeout;
				timeout.tv_sec = 5;
				timeout.tv_usec = 0;
				fd_set readfd;

				FD_ZERO(&readfd);

				FD_SET(sock, &readfd);

				int32_t ret = select(sock + 1, &readfd, NULL, NULL, &timeout);
				if (ret < 0)
					return -2;
			}
		}
		else if (RTP_AVP_TCP == sess->trans) {
			RtspHandleTcpConnect((void *)sess);
		}
	}
    sess->state = RTSP_KEEPALIVE;
    return True;
}

static int32_t RtspSendKeepAliveCommand(RtspSession *sess)
{
    if (True == RtspCommandIsSupported(RTSP_GET_PARAMETER, sess)){
        RtspGetParameterCommand(sess);
    }else{
        RtspOptionsCommand(sess);
		sess->state = RTSP_KEEPALIVE;//reset the status
    }

    return True;
}

int32_t RtspKeepAliveCommand(RtspSession *sess)
{
    struct timeval now;
    gettimeofday(&now, NULL);

    if (now.tv_sec - sess->last_cmd_time.tv_sec > sess->timeout-5){
#ifdef RTSP_DEBUG
    printf("+++++++++++++++++++  Keep alive: command  ++++++++++++++++++++++++++\n");
#endif
        RtspSendKeepAliveCommand(sess);
        sess->last_cmd_time = now;
    }

    return True;
}

static int32_t RtspSendGetParameterCommand(RtspSession *sess)
{
    int32_t size = sizeof(sess->buffctrl.buffer);
    int32_t sock = sess->sockfd;
    char *buf = sess->buffctrl.buffer;
    memset(buf, '\0', size);
    int32_t num = snprintf(buf, size, CMD_GET_PARAMETER, sess->url, sess->cseq, sess->sessid);
    if (num < 0x00){
        fprintf(stderr, "%s : snprintf error!\n", __func__);
        return False;
    }
    num = TcpSendData(sock, buf, (uint32_t)num);
    if (num < 0){
        fprintf(stderr, "%s : Send Error\n", __func__);
        return False;
    }

    return True;
}

int32_t RtspGetParameterCommand(RtspSession *sess)
{
    int32_t num;
    int32_t size = sizeof(sess->buffctrl.buffer);
    int32_t sock = sess->sockfd;
    char *buf = sess->buffctrl.buffer;

#ifdef RTSP_DEBUG
    printf("+++++++++++++++++++  Get Parameter: command  ++++++++++++++++++++++++++\n");
#endif
    if (False == RtspSendGetParameterCommand(sess))
        return False;

#ifdef RTSP_DEBUG
    printf("GET_PARAMETER Request: %s\n", buf);
#endif
    memset(buf, '\0', size);
    num = RtspReceiveResponse(sock, &sess->buffctrl);
    if (num <= 0) {
        fprintf(stderr, "Error: Server did not respond properly, closing...");
        return False;
    }


#ifdef RTSP_DEBUG
    printf("GET PARAMETER Reply: %s\n", buf);
#endif
    if (False == RtspCheckResponseStatus(buf))
        return False;
    return True;
}


static int32_t RtspSendTeardownCommand(RtspSession *sess)
{
    int32_t size = sizeof(sess->buffctrl.buffer);
    int32_t sock = sess->sockfd;
    char *buf = sess->buffctrl.buffer;
    memset(buf, '\0', size);
    int32_t num = snprintf(buf, size, CMD_TEARDOWN, sess->url, sess->cseq, sess->sessid);
    if (num < 0x00){
        fprintf(stderr, "%s : snprintf error!\n", __func__);
        return False;
    }
    num = TcpSendData(sock, buf, (uint32_t)num);
    if (num < 0){
        fprintf(stderr, "%s : Send Error\n", __func__);
        return False;
    }

    return True;
}

int32_t RtspTeardownCommand(RtspSession *sess)
{
    int32_t num;
    int32_t size = sizeof(sess->buffctrl.buffer);
    int32_t sock = sess->sockfd;
    char *buf = sess->buffctrl.buffer;

#ifdef RTSP_DEBUG
    printf("++++++++++++++++ TEARDOWN: command ++++++++++++++++++++++++++++\n");
#endif
    if (False == RtspSendTeardownCommand(sess))
        return False;

#ifdef RTSP_DEBUG
    printf("TEARDOWN Request: %s\n", buf);
#endif

    memset(buf, '\0', size);
    num = RtspReceiveResponse(sock, &sess->buffctrl);
    if (num <= 0) {
        fprintf(stderr, "Error: Server did not respond properly, closing...");
        return False;
    }

#ifdef RTSP_DEBUG
    printf("TEARDOWN Reply: %s\n", buf);
#endif
    if (False == RtspCheckResponseStatus(buf))
        return False;
    sess->state = RTSP_QUIT;
    return True;
}

static RtspCmdHdl rtspcmdhdl[] = {{RTSP_OPTIONS, RtspOptionsCommand},
                                {RTSP_DESCRIBE, RtspDescribeCommand},
                                {RTSP_SETUP, RtspSetupCommand},
                                {RTSP_PLAY, RtspPlayCommand},
                                {RTSP_GET_PARAMETER, RtspGetParameterCommand},
                                {RTSP_TEARDOWN, RtspTeardownCommand},
                                {RTSP_KEEPALIVE, RtspKeepAliveCommand}};

int32_t RtspStatusMachine(RtspSession *sess)
{
    int32_t size = sizeof(rtspcmdhdl)/sizeof(RtspCmdHdl);
    int32_t idx  = 0x00;

    for (idx = 0x00; idx < size; idx++){
        if (sess->state == rtspcmdhdl[idx].cmd){
            if (False == rtspcmdhdl[idx].handle(sess)){
                fprintf(stderr, "Error: Command wasn't supported.\n");
                return False;
            }
            RtspIncreaseCseq(sess);
        }
    }

    return True;
}
