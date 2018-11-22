#ifndef _RTSP_COMMON_H_
#define  _RTSP_COMMON_H_

#include "rtsp_type.h"
#include "rtsp.h"

#define CLIENT_PORT_FLAG 0x0A
#define SERVER_PORT_FLAG 0x0B

typedef struct CMD_TABLE{
    char cmd[64];
    int32_t key;
}CmdTbl;

int32_t ParseTimeout(char *buf, uint32_t size, RtspSession *sess);
int32_t ParseUdpPort(char *buf, uint32_t size, RtspSession *sess);
int32_t ParseUdpPort2(char *buf, uint32_t size, RtspSession *sess);
int32_t ParseInterleaved(char *buf, uint32_t num, RtspSession *sess);
int32_t ParseSessionID(char *buf, uint32_t size, RtspSession *sess);
int32_t ParseSdpProto(char *buf, uint32_t size, RtspSession *sess);
void GetSdpVideoTransport(char *buf, uint32_t size, RtspSession *sess);
void GetSdpVideoAcontrol(char *buf, uint32_t size, RtspSession *sess);
void GetSdpAudioTransport(char *buf, uint32_t size, RtspSession *sess);
void GetSdpAudioAcontrol(char *buf, uint32_t size, RtspSession *sess);
void RtspIncreaseCseq(RtspSession *sess);
void ParseOptionsPublic(char *buf, uint32_t size, RtspSession *sess);
int32_t RtspCommandIsSupported(int32_t key, RtspSession *sess);

#endif
