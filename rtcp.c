#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <arpa/inet.h>
#endif

#include "rtcp.h"
#include "utils.h"
#include "rtp.h"


static int32_t CreateSDES(char *buf, uint32_t size, RtpSession *sess);
static void ParseSenderDescribe(char *buf, uint32_t len, char *sdes);
static void ParseSenderReport(char *buf, uint32_t len, char *srh, RtpStats *stats);
static void ParseSenderReport(char *buf, uint32_t len, char *srh, RtpStats *stats)
{
    RtcpSR *rsr = (RtcpSR *)srh;
    SenderReport *sr = (SenderReport *)&(rsr->sr);
    char *ptr = buf;

    rsr->ssrc = htonl(GET_32((unsigned char *)ptr));
    ptr += 4;
    sr->ntp_timestamp_msw = GET_32((unsigned char *)ptr);
    ptr += 4;
    sr->ntp_timestamp_lsw = GET_32((unsigned char *)ptr);
    ptr += 4;
    sr->rtp_timestamp = GET_32((unsigned char *)ptr);
    ptr += 4;
    sr->senders_packet_count = GET_32((unsigned char *)ptr);
    ptr += 4;
    sr->senders_octet_count = GET_32((unsigned char *)ptr);
    ptr += 4;

    if ((ptr-buf) > len){
        fprintf(stderr, "%s: length error!\n", __func__);
        return;
    }

    struct timeval now;
    gettimeofday(&now, NULL);
    stats->last_rcv_SR_ts = ((sr->ntp_timestamp_msw&0xFFFF)<<16) |\
                            (sr->ntp_timestamp_lsw >> 16);
    stats->last_rcv_SR_time.tv_sec  = now.tv_sec;
    stats->last_rcv_SR_time.tv_usec = now.tv_usec;
    stats->rtp_identifier = rsr->ssrc;

#ifdef RTSP_DEBUG
    if (0) {
        printf("Timestamp MSW   : 0x%x (%u)\n", sr->ntp_timestamp_msw, sr->ntp_timestamp_msw);
        printf("Timestamp LSW   : 0x%x (%u)\n", sr->ntp_timestamp_lsw, sr->ntp_timestamp_lsw);
        printf("RTP timestamp  : 0x%x (%u)\n", sr->rtp_timestamp, sr->rtp_timestamp);
        printf("Sender Packet Count : %u\n", sr->senders_packet_count);
        printf("Sender Octet Count : %u\n", sr->senders_octet_count);
    }
#endif

    return;
}

static void ParseSenderDescribe(char *buf, uint32_t len, char *sdes)
{
#if 0
    SDES *sd = (SDES *)(sdes);
    char *ptr = buf;


    sd->identifier = GET_32(ptr);
    ptr += 4;

#ifdef RTSP_DEBUG
    if (1){
        printf("     ID       : %u\n", sd->identifier);
        /*printf("     Type     : %i\n", sd->start);*/
        /*printf("     Length   : %i\n", sd->length);*/
        /*printf("     Type 2   : %i\n", sd->end);*/
    }
#endif
#endif

    return;
}

uint32_t ParseRtcp(char *buf, uint32_t len, RtpStats *stats)
{
    uint32_t i = 0;
    char *ptr = buf;
    struct timeval now;
    gettimeofday(&now, NULL);

    while (i < len) {
        RtcpHeader *rtcph = (RtcpHeader *)(ptr);
#if 1
        /* RTCP */
        rtcph->version   = (buf[i] >> 6)&0x03;
        rtcph->padbit    = ((buf[i] & 0x20) >> 5)&0x01;
        rtcph->rc        = buf[i] & 0x1F;
#endif
#ifdef RTSP_DEBUG
        if (0){
            printf("RTCP Version  : %i\n", rtcph->version);
            printf("     Padding  : %i\n", rtcph->padbit);
            printf("     RC       : %i\n", rtcph->rc);
            printf("     Type     : %i\n", rtcph->type);
            printf("     Length   : %i\n", GET_16(rtcph->length));
        }
#endif
        ptr += sizeof(RtcpHeader);
        uint32_t length = GET_16(rtcph->length)*4;
        if (rtcph->type == RTCP_SR){
            RtcpSR rsr;
            ParseSenderReport(ptr, length, (char *)&(rsr), stats);
        }else if (rtcph->type == RTCP_SDES){/* source definition */
            SDES sdes;
            ParseSenderDescribe(ptr, length, (char *)&sdes);
        }else if (rtcph->type == RTCP_BYE){
            return RTCP_BYE;
        }
        ptr += length;
        i += length + sizeof(RtcpHeader);
    }

    return RTCP_SR;
}


static void InitRtcpHeader(RtcpHeader *ch, uint32_t type, uint32_t rc, uint32_t bytes_len)
{
    char *ptr = (char *)ch;
    ptr[0] = 0x81;
    ch->type = type;
    PUT_16(&ch->length[0], (bytes_len/4)-1);
	return;
}

static uint32_t InitRtcpReceiveReport(char *buf, uint32_t size)
{
    RtcpRR *rrr=(RtcpRR *)buf;
    if (size < sizeof(RtcpRR)) return 0;
    InitRtcpHeader(&rrr->hdr, RTCP_RR, 1, sizeof(RtcpRR));
    rrr->ssrc = htonl(RTCP_SSRC);
    return sizeof(RtcpRR);
}

uint32_t RtcpReceiveReport(char *buf, uint32_t len, RtpSession *sess)
{
    RtpStats *rtpst = (RtpStats *)(&sess->stats);

    /* init Receive Report */
    InitRtcpReceiveReport(buf, len);

	RtcpRR *rrr=(RtcpRR *)buf;
    /*
     * Calcs for expected and lost packets
     */
    uint32_t extended_max;
    uint32_t expected;
    extended_max = rtpst->rtp_received + rtpst->highest_seq;
    expected = extended_max - rtpst->first_seq + 1;
    rtpst->rtp_cum_lost = expected - rtpst->rtp_received - 1;
#if 0
    /* Fraction */
    uint32_t expected_interval;
    uint32_t received_interval;
    uint32_t lost_interval;
    uint8_t fraction;

    expected_interval = expected - rtpst->rtp_expected_prior;
    rtpst->rtp_expected_prior = expected;

    received_interval = rtpst->rtp_received - rtpst->rtp_received_prior;
    rtpst->rtp_received_prior = rtpst->rtp_received;
    lost_interval = expected_interval - received_interval;
    if (expected_interval == 0 || lost_interval <= 0){
        fraction = 0;
    }else{
        fraction = (lost_interval << 8) / expected_interval;
    }
#endif
    ReceiveReport *rr = &rrr->rr;
    rr->ssrc = sess->stats.rtp_identifier;
    rr->fl_cnpl = 0xFFFFFFFF;

    /* RTCP: SSRC Contents: Extended highest sequence */
    PUT_16(rr->cycle, 0x01);
    PUT_16(rr->highestseq, rtpst->highest_seq);

    /* RTCP: SSRC Contents: interarrival jitter */
    rr->interarrival_jitter = rtpst->jitter;

    /* RTCP: SSRC Contents: Last SR timestamp */
    rr->lsr = rtpst->last_rcv_SR_ts;

    /* RTCP: SSRC Contents: Timestamp delay */

    struct timeval now;
    double delay;
    if (rtpst->last_rcv_SR_ts != 0) {
        gettimeofday(&now,NULL);
        delay= (now.tv_sec - rtpst->last_rcv_SR_time.tv_sec) +
               ((now.tv_usec - rtpst->last_rcv_SR_time.tv_usec)*1e-6);
        delay= (delay * 65536);
        rtpst->delay_snc_last_SR = (uint32_t) delay;
    }
    rr->delay_snc_last_sr = rtpst->delay_snc_last_SR;

    uint32_t size = CreateSDES(buf+sizeof(RtcpRR), len-sizeof(RtcpRR), sess);
    return sizeof(RtcpRR)+size;
}

static int32_t CreateSDES(char *buf, uint32_t size, RtpSession *sess)
{
    RtcpHeader *rhdr = (RtcpHeader *)buf;
    if (size < sizeof(RtcpHeader)) return 0;
    InitRtcpHeader(rhdr, RTCP_SDES, 1, sizeof(RtcpHeader));

    /*identifier*/
    char *ptr = buf + sizeof(RtcpHeader);
    PUT_32((unsigned char *)ptr, htonl(sess->stats.rtp_identifier));
    ptr += 4;

    /* SDES ITEMS */
    /* SDES CANME */
    *ptr = 0x01;
    ptr += 1;

    /* SDES Length */
    *ptr = 0x0a;
    ptr += 1;

    /* SDES TEXT */
    char text[] = "1234567890";
    memcpy((void *)ptr, (const void *)text, strlen(text));
    ptr += strlen(text);

#if 0
    /* SDES TOOL */
    *ptr = 0x06;
    ptr += 1;

    /* SDES length */
    char tool[] = "90n6h.0.1";
    *ptr = strlen(tool);
    ptr += 1;

    /* SDES TEXT */
    memcpy((void *)ptr, (const void *)tool, strlen(tool));
    ptr += strlen(tool);
#endif

    /* SDES END */
    *ptr = 0x00;
    ptr += 1;

    PUT_16(&rhdr->length[0], ((ptr-buf+3)/4)-1);

    return (ptr-buf);
}
