#ifndef _RTSP_H_
#define _RTSP_H_
#ifdef __cplusplus
extern "C"
{
#endif



#include <time.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <WinSock2.h>
#endif
#include "port.h"
#include "rtsp_type.h"

typedef enum{
    RTSP_START,
    RTSP_OPTIONS = 1,
    RTSP_DESCRIBE = 2,
    RTSP_SETUP = 4,
    RTSP_PLAY = 8,
    RTSP_PAUSE = 16,
    RTSP_GET_PARAMETER = 32,
    RTSP_SET_PARAMETER = 64,
    RTSP_REDIRECT = 128,
    RTSP_TEARDOWN = 256,
	RTSP_KEEPALIVE = 512,
    RTSP_QUIT
}EN_RTSP_STATUS;

typedef enum{
    RTP_AVP_TCP,
    RTP_AVP_UDP
}EN_TRANSPORTS;

typedef struct Public_Table{
    char cmd[16];
    uint32_t key;
}PublicTbl;

typedef struct RTP_TCP{
    char  start;          /* interleaved start */
    char  end;            /* interleaved end   */
    char  reserve[3];
}RtpTcp;

typedef struct RTP_UDP{
    uint32_t cport_from;    /* client_port from */
    uint32_t cport_to;      /* client port to   */
    uint32_t sport_from;    /* server port from */
    uint32_t sport_to;      /* server port to   */
    char unicast;
    char mode;
    char reserve[2];
}RtpUdp;

typedef struct VIDEO_MEDIA{
    char control[128];
}VideoMedia;

typedef struct AUDIO_MEDIA{
    char control[128];
}AudioMedia;


typedef void(*RtspClientErrorCb)(void * s);
typedef void(*RtspPlayStartCb)(void * s);
typedef int(*RtspSetupCb)(int chn, int payloadType);
typedef int(*RtspPlayCb)(int chn, char *type, char * p_buf, int size, void * param);
typedef int(*RtspTearDownCb)(int chn);

typedef struct RH{
	RtspClientErrorCb clientErrorCb;
	RtspPlayStartCb playStartCb;
	RtspSetupCb		setupCb;
	RtspPlayCb		playCb;
	RtspTearDownCb  teardownCb;
}RtspHandle;



typedef struct RTSPSESSION{
	uint32_t channel;
    uint32_t port;
    int32_t  sockfd;
    int32_t  cseq;
    uint32_t timeout;
    int32_t  cmdstats;
    uint32_t state;

    uint32_t packetization; /* Packetization mode from SDP data */
    union{
        RtpUdp    udp;
        RtpTcp    tcp;
    }transport;
	union {
		RtpUdp    udp;
		RtpTcp    tcp;
	}transport2;
    PublicTbl     *pubtbl;
    void *     rtpsess;
    AudioMedia    amedia;
    VideoMedia    vmedia;
    BufferControl buffctrl;
    struct timeval last_cmd_time;
    char  sessid[32];
    char  url[128];
    char  username[128];
    char  password[128];
    char  ip[16];
    char  trans;      /* RTP/AVP/UDP or RTP/AVP/TCP */
	char  trans2;      /* RTP/AVP/UDP or RTP/AVP/TCP */
    char  reserve[2];
	os_thread_t rtpid;
	RtspClientErrorCb ec;
	RtspPlayStartCb pc;
	RtspSetupCb		setupCb;
	RtspPlayCb		playCb;
	RtspTearDownCb  teardownCb;
	
	void * puser;
	char  sdp[2048];
}RtspSession;

typedef struct RTSP_COMMOND_HANDLE{
    int32_t cmd;
    int32_t (*handle)(RtspSession *);
}RtspCmdHdl;

int32_t RtspOptionsCommand(RtspSession *sess);
int32_t RtspDescribeCommand(RtspSession *sess);
int32_t RtspSetupCommand(RtspSession *sess);
int32_t RtspPlayCommand(RtspSession *sess);
int32_t RtspTeardownCommand(RtspSession *sess);
int32_t RtspStatusMachine(RtspSession *sess);
int32_t RtspGetParameterCommand(RtspSession *sess);
int32_t RtspKeepAliveCommand(RtspSession *sess);



#ifdef __cplusplus
}
#endif

#endif
