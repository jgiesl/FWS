
#ifndef __PLATFORM__
#define __PLATFORM__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
	#include <winsock2.h>
	#define THREAD_FUNCTION DWORD WINAPI

	typedef signed __int8     int8_t;
	typedef signed __int16    int16_t;
	typedef signed __int32    int32_t;
	typedef unsigned __int8   uint8_t;
	typedef unsigned __int16  uint16_t;
	typedef unsigned __int32  uint32_t;

#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netinet/tcp.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <pthread.h>
	#define THREAD_FUNCTION void*
	#define SOCKET int
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#define SOCKADDR struct sockaddr
	#define SOCKADDR_IN struct sockaddr_in
	#define closesocket close
#endif

#ifdef __cplusplus
}
#endif

#endif
