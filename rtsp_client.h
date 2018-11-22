#ifndef _RTSP_Client_H_
#define _RTSP_Client_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "rtsp_type.h"
#include "rtsp.h"

typedef struct RTSP_VIDEO_INFO{
	int videoid;
}VideoInfo;

#if 0
v=0                     //以下都是sdp信息
o=- 0 0 IN IPV4 127.0.0.1
t=0 0
s=No Name
a=tool:libavformat
m=video 0 RTP/AVP 96 //m表示媒体描述，下面是对会话中视频通道的媒体描述
b=AS:2000
a=rtpmap:96 MP4V-ES/90000
a=fmtp:96 profile-level-id=1
a=control:streamid=0 //streamid＝0表示视频流用的是通道0
#endif
typedef struct SDP_MEDIA{
    int32_t port;
    int32_t num;    /* number of ports */
    char type;    /*video or audio*/
    char protocl; /* RTP/AVP or RTP/TCP/AVP */
    char format;
    char reserve;
}SdpMedia;

typedef struct SDP_RTPMAP{
    int32_t payload;
    char encodename[16];
    int32_t clockrate;
    int32_t param; /* 视频流没有编码参数 */
}SdpRtpMap;

typedef struct SDP_ATTRIBUTES{
    int32_t type;
    int32_t value;
}SdpAttributes;

typedef struct RTSP_SDP{
    char *version;
    char *owner;
    char *name;
    SdpMedia        *media;
    SdpRtpMap       *map;
    SdpAttributes   *att;
}RtspSdp;

typedef struct RTSP_CLIENT_SESSION{
    RtspSession sess;
    RtspSdp     sdp;
    int32_t     quit;
	os_thread_t rtspid;
}RtspClientSession;


uint32_t ParseUrl(char *url, RtspClientSession *cses);
void* RtspEventLoop(void* args);
RtspClientSession* InitRtspClientSession();

int32_t ParseRtspUrl(char *url, RtspSession *sess);
int32_t isRtspClientSessionQuit(RtspClientSession *rcsess);
void SetRtspClientSessionQuit(RtspClientSession *rcsess);

int rtsp_client_init();
RtspClientSession * startRtspClient(int channel, char * url, RtspHandle handle, void * puser);
void closeRtspClient(RtspClientSession *csess);

#ifdef __cplusplus
}
#endif

#endif
