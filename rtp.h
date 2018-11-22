#ifndef _RTP_H_
#define _RTP_H_

#include <time.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <WinSock2.h>
#endif

#include "rtsp_type.h"



#define FRAME_MAX_SIZE              (1920*1080)
#define RTP_TCP_MAGIC               (0x24)
#define RTP_FREQ                    (90000)
#define PACKET_BUFFER_END           ((unsigned int)0x00000000)
#define MAX_RTP_PKT_LENGTH          (1400)
#define H264_PAYLOAD                (96)

#ifdef IN_LINPHONE_
#include "ortp/rtp.h"
#else
/******************************************************************
RTP_FIXED_HEADER
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           synchronization source (SSRC) identifier            |
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|            contributing source (CSRC) identifiers             |
|                             ....                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

******************************************************************/

typedef struct rtp_header {
    uint32_t version:2;     /* protocol version */
    uint32_t padding:1;     /* padding flag */
    uint32_t extension:1;   /* header extension flag */
    uint32_t cc:4;          /* CSRC count */
    uint32_t marker:1;      /* marker bit */
    uint32_t pt:7;          /* payload type */
    uint32_t seq:16;            /* sequence number */
    uint32_t ts;                /* timestamp */
    uint32_t ssrc;              /* synchronization source */
}RtpHeader;

/******************************************************************
NALU_HEADER
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
******************************************************************/
typedef struct {
    //byte 0
	unsigned char TYPE:5;
    unsigned char NRI:2;
	unsigned char F:1;
} NALU_HEADER; /* 1 byte */


/******************************************************************
FU_INDICATOR
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
******************************************************************/
typedef struct {
    //byte 0
    unsigned char TYPE:5;
	unsigned char NRI:2;
	unsigned char F:1;
} FU_INDICATOR; /*1 byte */


/******************************************************************
FU_HEADER
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|R|  Type   |
+---------------+
******************************************************************/
typedef struct {
    //byte 0
    unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;
} FU_HEADER; /* 1 byte */


typedef struct RTP_OVER_TCP{
    unsigned char magic;
    unsigned char ch;
    unsigned char len[2];
}RtpOverTcp;

/* Enumeration of H.264 NAL unit types */
enum {
    NAL_TYPE_UNDEFINED = 0,
    NAL_TYPE_SINGLE_NAL_MIN	= 1,
    NAL_TYPE_SINGLE_NAL_MAX	= 23,
    NAL_TYPE_STAP_A		= 24,
    NAL_TYPE_FU_A		= 28,
};

typedef struct Rtp_Stats{
    uint32_t first_seq;         /* first sequence                   */
    uint32_t highest_seq;       /* highest sequence                 */
    uint32_t rtp_received;      /* RTP sequence number received     */
    uint32_t rtp_identifier;    /* source identifier                */
    uint32_t rtp_ts;            /* RTP timestamp                    */
    uint32_t rtp_cum_lost;       /* RTP cumulative packet lost       */
    uint32_t rtp_expected_prior;/* RTP expected prior               */
    uint32_t rtp_received_prior;/* RTP received prior               */
    uint32_t transit;           /* Transit time. RFC3550 A.8        */
    uint32_t jitter;            /* Jitter                           */
    uint32_t lst;
    uint32_t last_dlsr;         /* Last DLSR                        */
    uint32_t last_rcv_SR_ts;    /* Last arrival in RTP format       */
    uint32_t delay_snc_last_SR; /* Delay sinde last SR              */
    struct timeval  last_rcv_SR_time;           /* Last SR arrival                  */
    struct timeval  last_rcv_time;
    double rtt_frac;
}RtpStats;

typedef struct Rtp_Session{
    uint32_t seq;
    uint32_t ssrc;
    struct timeval rtcptv;
    struct sockaddr_in addrfrom;
    struct sockaddr_in addrto;
    RtpStats stats;
}RtpSession;
#endif

uint32_t GetRtpHeaderLength(char *buf, uint32_t size);
int32_t CheckRtpSequence(char *buf, void* args);
int32_t CheckRtpHeaderMarker(char *buf, uint32_t size);
unsigned int UnpackRtpNAL(char *buf, uint32_t size, char *framebuf, uint32_t framelen);
void ParseRtp(char *buf, uint32_t size, RtpSession *sess);



#endif
