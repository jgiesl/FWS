
#ifndef __CLIENT__
#define __CLIENT__

#include "platform.h"

#include "connection.h"
#include "message.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct client_t client_t;

struct client_t
{
	connection_t* connection;
	int status;
};

client_t* client_create(char* address, int port);
int client_destroy(client_t* client);

message_t* client_recv_msg(client_t* client);
int client_send_msg(client_t* client, message_t* message);

#ifdef __cplusplus
}
#endif

#endif
