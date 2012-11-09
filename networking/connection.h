
#ifndef __CONNECTION__
#define __CONNECTION__

#include "platform.h"

#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	SOCKET socket;
	SOCKADDR* info;
}connection_t;

connection_t* connection_create();
void connection_destroy_callback(void* connection_);
int connection_destroy(connection_t* connection);
message_t* connection_recv(connection_t* connection);
int connection_send(connection_t* connection, message_t* message);

#ifdef __cplusplus
}
#endif

#endif
