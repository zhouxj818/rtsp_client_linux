#ifndef _TPOOL_H
#define _TPOOL_H

#include <stdio.h>
#include "port.h"
#ifndef WIN32
#include <pthread.h>
#endif

#include <stdlib.h>

typedef void *(*FUNC)(void *);

os_thread_t RtspCreateThread(FUNC func, void *args);
int32_t TrykillThread(os_thread_t id);
#endif
