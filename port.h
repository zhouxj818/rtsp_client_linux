/*
  The Os library is an RTP (Realtime Transport Protocol - rfc3550) stack.
  Copyright (C) 2001  Simon MORLAT simon.morlat@linphone.org

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* this file is responsible of the portability of the stack */

#ifndef os_PORT_H
#define os_PORT_H


#if !defined(WIN32) && !defined(_WIN32_WCE)
/********************************/
/* definitions for UNIX flavour */
/********************************/

#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef __linux
#include <stdint.h>
#endif


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if defined(_XOPEN_SOURCE_EXTENDED) || !defined(__hpux)
#include <arpa/inet.h>
#endif



#include <sys/time.h>

#ifdef os_INET6
#include <netdb.h>
#endif

typedef int os_socket_t;
typedef pthread_t os_thread_t;
typedef pthread_mutex_t os_mutex_t;
typedef pthread_cond_t os_cond_t;

#ifdef __INTEL_COMPILER
#pragma warning(disable : 111)		// statement is unreachable
#pragma warning(disable : 181)		// argument is incompatible with corresponding format string conversion
#pragma warning(disable : 188)		// enumerated type mixed with another type
#pragma warning(disable : 593)		// variable "xxx" was set but never used
#pragma warning(disable : 810)		// conversion from "int" to "unsigned short" may lose significant bits
#pragma warning(disable : 869)		// parameter "xxx" was never referenced
#pragma warning(disable : 981)		// operands are evaluated in unspecified order
#pragma warning(disable : 1418)		// external function definition with no prior declaration
#pragma warning(disable : 1419)		// external declaration in primary source file
#pragma warning(disable : 1469)		// "cc" clobber ignored
#endif

#ifdef __cplusplus
extern "C"
{
#endif

int __os_thread_join(os_thread_t thread, void **ptr);
int __os_thread_create(pthread_t *thread, pthread_attr_t *attr, void * (*routine)(void*), void *arg);

#ifdef __cplusplus
}
#endif

#define os_thread_create	__os_thread_create
#define os_thread_join	__os_thread_join
#define os_thread_exit	pthread_exit
#define os_mutex_init		pthread_mutex_init
#define os_mutex_lock		pthread_mutex_lock
#define os_mutex_unlock	pthread_mutex_unlock
#define os_mutex_destroy	pthread_mutex_destroy
#define os_cond_init		pthread_cond_init
#define os_cond_signal	pthread_cond_signal
#define os_cond_broadcast	pthread_cond_broadcast
#define os_cond_wait		pthread_cond_wait
#define os_cond_destroy	pthread_cond_destroy

#define SOCKET_OPTION_VALUE	void *
#define SOCKET_BUFFER		void *

#define getSocketError() strerror(errno)
#define getSocketErrorCode() (errno)

#define os_log10f(x)	log10f(x)


#else
/*********************************/
/* definitions for WIN32 flavour */
/*********************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>

#ifdef _MSC_VER
#pragma push_macro("_WINSOCKAPI_")
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

typedef  unsigned __int64 uint64_t;
typedef  __int64 int64_t;
typedef  unsigned short uint16_t;
typedef  unsigned int uint32_t;
typedef  int int32_t;
typedef  unsigned char uint8_t;
typedef __int16 int16_t;

#else
#include <stdint.h> /*provided by mingw32*/
#endif

#define vsnprintf	_vsnprintf
#define srandom		srand
#define random		rand


typedef SOCKET os_socket_t;
typedef HANDLE os_cond_t;
typedef HANDLE os_mutex_t;
typedef HANDLE os_thread_t;

#define os_thread_create	WIN_thread_create
#define os_thread_join	WIN_thread_join
#define os_thread_exit(arg)
#define os_mutex_init		WIN_mutex_init
#define os_mutex_lock		WIN_mutex_lock
#define os_mutex_unlock	WIN_mutex_unlock
#define os_mutex_destroy	WIN_mutex_destroy
#define os_cond_init		WIN_cond_init
#define os_cond_signal	WIN_cond_signal
#define os_cond_broadcast	WIN_cond_broadcast
#define os_cond_wait		WIN_cond_wait
#define os_cond_destroy	WIN_cond_destroy


#ifdef __cplusplus
extern "C"
{
#endif

int WIN_mutex_init(os_mutex_t *m, void *attr_unused);
int WIN_mutex_lock(os_mutex_t *mutex);
int WIN_mutex_unlock(os_mutex_t *mutex);
int WIN_mutex_destroy(os_mutex_t *mutex);
int WIN_thread_create(os_thread_t *t, void *attr_unused, void *(*func)(void*), void *arg);
int WIN_thread_join(os_thread_t thread, void **unused);
int WIN_cond_init(os_cond_t *cond, void *attr_unused);
int WIN_cond_wait(os_cond_t * cond, os_mutex_t * mutex);
int WIN_cond_signal(os_cond_t * cond);
int WIN_cond_broadcast(os_cond_t * cond);
int WIN_cond_destroy(os_cond_t * cond);
#define logDebug printf
#ifdef __cplusplus
}
#endif

#define SOCKET_OPTION_VALUE	char *
#define inline			__inline

#if defined(_WIN32_WCE)

#define os_log10f(x)		(float)log10 ((double)x)

#ifdef assert
	#undef assert
#endif /*assert*/
#define assert(exp)	((void)0)

#ifdef errno
	#undef errno
#endif /*errno*/
#define  errno GetLastError()
#ifdef strerror
		#undef strerror
#endif /*strerror*/
const char * os_strerror(DWORD value);
#define strerror os_strerror


#else /*_WIN32_WCE*/

#define os_log10f(x)	log10f(x)

#endif

#endif

typedef unsigned char bool_t;
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

#ifdef __cplusplus
extern "C"{
#endif

void* os_malloc(size_t sz);
void os_free(void *ptr);
void* os_realloc(void *ptr, size_t sz);
void* os_malloc0(size_t sz);
char * os_strdup(const char *tmp);

/*override the allocator with this method, to be called BEFORE os_init()*/
typedef struct _OsMemoryFunctions{
	void *(*malloc_fun)(size_t sz);
	void *(*realloc_fun)(void *ptr, size_t sz);
	void (*free_fun)(void *ptr);
}OsMemoryFunctions;

void os_set_memory_functions(OsMemoryFunctions *functions);

#define os_new(type,count)	(type*)os_malloc(sizeof(type)*(count))
#define os_new0(type,count)	(type*)os_malloc0(sizeof(type)*(count))

int close_socket(os_socket_t sock);
int set_non_blocking_socket(os_socket_t sock);

char *os_strndup(const char *str,int n);
char *os_strdup_printf(const char *fmt,...);
char *os_strdup_vprintf(const char *fmt, va_list ap);

int os_file_exist(const char *pathname);

/* portable named pipes  and shared memory*/
#if !defined(_WIN32_WCE)
#ifdef WIN32
typedef HANDLE os_pipe_t;
#define os_PIPE_INVALID INVALID_HANDLE_VALUE
#else
typedef int os_pipe_t;
#define os_PIPE_INVALID (-1)
#endif

os_pipe_t os_server_pipe_create(const char *name);
/*
 * warning: on win32 os_server_pipe_accept_client() might return INVALID_HANDLE_VALUE without
 * any specific error, this happens when os_server_pipe_close() is called on another pipe.
 * This pipe api is not thread-safe.
*/
os_pipe_t os_server_pipe_accept_client(os_pipe_t server);
int os_server_pipe_close(os_pipe_t spipe);
int os_server_pipe_close_client(os_pipe_t client);

os_pipe_t os_client_pipe_connect(const char *name);
int os_client_pipe_close(os_pipe_t sock);

int os_pipe_read(os_pipe_t p, uint8_t *buf, int len);
int os_pipe_write(os_pipe_t p, const uint8_t *buf, int len);

void *os_shm_open(unsigned int keyid, int size, int create);
void os_shm_close(void *memory);

#endif

#ifdef __cplusplus
}

#endif


#if (defined(WIN32) || defined(_WIN32_WCE)) && !defined(os_STATIC)
#ifdef os_EXPORTS
   #define VAR_DECLSPEC    __declspec(dllexport)
#else
   #define VAR_DECLSPEC    __declspec(dllimport)
#endif
#else
   #define VAR_DECLSPEC    extern
#endif


/***************/
/* logging api */
/***************/

typedef enum {
	os_DEBUG = 1,
	os_MESSAGE = 1 << 1,
	os_WARNING = 1 << 2,
	os_ERROR = 1 << 3,
	os_FATAL = 1 << 4,
	os_LOGLEV_END = 1 << 5
} OsLogLevel;


typedef void(*OsLogFunc)(OsLogLevel lev, const char *fmt, va_list args);

void os_set_log_file(FILE *file);
void os_set_log_handler(OsLogFunc func);

VAR_DECLSPEC OsLogFunc os_logv_out;

extern unsigned int __os_log_mask;

#define os_log_level_enabled(level)	(__os_log_mask & (level))

#if !defined(WIN32) && !defined(_WIN32_WCE)
#define os_logv(level,fmt,args) \
{\
	if (os_logv_out!=NULL && os_log_level_enabled(level)) \
		os_logv_out(level,fmt,args);\
	if ((level)==os_FATAL) abort();\
}while(0)
#else
void os_logv(int level, const char *fmt, va_list args);
#endif

void os_set_log_level_mask(int levelmask);

#ifdef os_DEBUG_MODE
static inline void os_debug(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	os_logv(os_DEBUG, fmt, args);
	va_end(args);
}
#else

#define os_debug(...)

#endif

#ifdef os_NOMESSAGE_MODE

#define os_log(...)
#define os_message(...)
#define os_warning(...)

#else

static inline void os_log(OsLogLevel lev, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	os_logv(lev, fmt, args);
	va_end(args);
}

static inline void os_message(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	os_logv(os_MESSAGE, fmt, args);
	va_end(args);
}

static inline void os_warning(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	os_logv(os_WARNING, fmt, args);
	va_end(args);
}

#endif

static inline void os_error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	os_logv(os_ERROR, fmt, args);
	va_end(args);
}

static inline void os_fatal(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	os_logv(os_FATAL, fmt, args);
	va_end(args);
}

#ifdef WIN32
void * memmem(const void *l, size_t l_len, const void *s, size_t s_len);
#endif

#endif


