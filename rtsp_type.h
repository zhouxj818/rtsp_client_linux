#ifndef _RTSP_TYPE_H_
#define _RTSP_TYPE_H_

#define True  0x01
#define False 0x00

#define RECV_BUFF_DEFAULT_LEN  (2048)
#define SEPERATOR         "\r\n\r\n"
#define VERSION           "0.1"
#define PROTOCOL_PREFIX   "rtsp://"
#define RTSP_PORT         554
#define RTSP_CLIENT_PORT  9500
#define RTSP_RESPONSE     "RTSP/1.0 "
#define CMD_OPTIONS       "OPTIONS rtsp://%s:%d RTSP/1.0\r\nCSeq: %i\r\nUser-Agent: s-rtsp\r\n\r\n"
#define CMD_DESCRIBE      "DESCRIBE %s RTSP/1.0\r\nCSeq: %i\r\nUser-Agent: s-rtsp\r\nAccept: application/sdp\r\n\r\n"
#define CMD_TCP_SETUP     "SETUP %s RTSP/1.0\r\nCSeq: %i\r\nUser-Agent: s-rtsp\r\nTransport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n"
#define CMD_UDP_SETUP     "SETUP %s RTSP/1.0\r\nCSeq: %i\r\nUser-Agent: s-rtsp\r\nTransport: RTP/AVP;unicast;client_port=%d-%d\r\n\r\n"
#define CMD_UDP_SETUP_S   "SETUP %s RTSP/1.0\r\nCSeq: %i\r\nUser-Agent: s-rtsp\r\nTransport: RTP/AVP;unicast;client_port=%d-%d\r\nSession: %s\r\n\r\n"
#define CMD_PLAY          "PLAY %s RTSP/1.0\r\nCSeq: %i\r\nUser-Agent: s-rtsp\r\nSession: %s\r\nRange: npt=0.00-\r\n\r\n"
#define CMD_GET_PARAMETER "GET_PARAMETER %s RTSP/1.0\r\nCSeq: %i\r\nSession: %s\r\n\r\n"
#define CMD_TEARDOWN      "TEARDOWN %s RTSP/1.0\r\nCSeq: %i\r\nSession: %s\r\n\r\n"
#define UDP_TRANSPORT     "RTP/AVP"
#define TCP_TRANSPORT     "RTP/AVP/TCP"
#define TCP_INTERLEAVED   "interleaved="
#define TIME_OUT          "timeout="
#define OPTIONS_PUBLIC    "Public: "

#define SETUP_SESSION      "Session: "
#define SETUP_CPORT        "client_port="
#define SETUP_SPORT        "server_port="

#define SDP_M_VIDEO        "m=video "
#define SDP_M_AUDIO        "m=audio "
#define SDP_A_RTPMAP       "a=rtpmap:"
#define SDP_A_FMTP         "a=fmtp:"
#define SDP_A_CONTROL      "a=control:"
#define CONTENT_LENGTH    "Content-Length: "
#define CONTENT_length    "Content-length: "

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef struct buffer_control{
    int32_t     len;
    int32_t     offset;
    char        buffer[RECV_BUFF_DEFAULT_LEN];
}BufferControl;

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define s_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                      \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - s_offsetof(type,member) );})


static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *newitem, struct list_head *prev,struct list_head *next)
{
    next->prev = newitem;
    newitem->next = next;
    newitem->prev = prev;
    prev->next = newitem;
}


static inline void list_add(struct list_head *newitem, struct list_head *head)
{
    __list_add(newitem, head, head->next);
}


static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}


#endif
