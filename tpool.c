#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#else
//#include <windows.h>
#endif
#include <stdio.h>
#include <errno.h>

#include "tpool.h"
#include "utils.h"

os_thread_t RtspCreateThread(FUNC func, void* args)
{
    os_thread_t id;
    if (0x00 != os_thread_create(&id, NULL, func, args)){
        fprintf(stderr, "pthread_create error!\n");
        return -1;
    }
    return id;
}

int32_t TrykillThread(os_thread_t id)
{
    int32_t ret =  0x00;
    do{
        ret = os_thread_join(id, 0x00);
        if (ESRCH == ret){
            break;
        }
    }while(1);

    return ret;
}


