#ifndef _RTSP_RESPONSE_H_
#define _RTSP_RESPONSE_H_

#include "rtsp_type.h"

#define SEPERATOR "\r\n\r\n"

int32_t RtspReceiveResponse(uint32_t sockfd, BufferControl *bctrl);
int32_t RtspCheckResponseStatus(char *buff);

#endif
