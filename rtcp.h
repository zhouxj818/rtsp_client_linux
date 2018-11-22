#ifndef _RTCP_H_
#define _RTCP_H_


#include <stdint.h>
#ifndef WIN32
#include <sys/time.h>
#endif

#include "rtsp_type.h"
#include "utils.h"
#include "rtp.h"

/* Payload types */
#define RTCP_SR     200   /*  sender report        */
#define RTCP_RR     201   /*  receiver report      */
#define RTCP_SDES   202   /*  source description   */
#define RTCP_BYE    203   /*  good bye             */
#define RTCP_APP    204   /*  application defined  */

/* Identification */
#define RTCP_SSRC   0x0c143e07


typedef struct RTCP_Header{
    unsigned char  version:2;
    unsigned char  padbit:1;
    unsigned char  rc:5; /*report count*/
    unsigned char  type;
    unsigned char  length[2];
}RtcpHeader;

typedef struct SOURCE_DESCRIBE{
    /* source definition */
    uint32_t identifier;
    unsigned char  start;
    unsigned char  length;
    uint32_t text;
    unsigned char  end;
}SDES;

typedef struct REPORT
{
    uint32_t ssrc;
    uint32_t fl_cnpl;/*fraction lost + cumulative number of packet lost*/
    unsigned char cycle[2];
    unsigned char highestseq[2]; /*extended highest sequence number received */
    uint32_t interarrival_jitter;
    uint32_t lsr; /*last SR */
    uint32_t delay_snc_last_sr; /*delay since last sr*/
}ReceiveReport;

/* SR or RR  packets */

typedef struct SENDER_REPORT
{
    uint32_t ntp_timestamp_msw;
    uint32_t ntp_timestamp_lsw;
    uint32_t rtp_timestamp;
    uint32_t senders_packet_count;
    uint32_t senders_octet_count;
}SenderReport;

typedef struct rtcp_sr{
    RtcpHeader hdr;
    uint32_t ssrc;
    SenderReport sr;
}RtcpSR;

typedef struct rtcp_rr{
    RtcpHeader  hdr;
    uint32_t ssrc;
    ReceiveReport   rr;
}RtcpRR;



uint32_t RtcpReceiveReport(char *buf, uint32_t len, RtpSession *sess);
uint32_t ParseRtcp(char *buf, uint32_t len, RtpStats *stats);
#endif
