
#ifndef __SERVER__
#define __SERVER__

#include "platform.h"

#include "connection.h"
#include "message.h"
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct server_t server_t;

struct server_t
{
	SOCKET socket;
	int status;
	fd_set read_fdset;

	vector_t* connections;  // list of pointers to connections
};

server_t* server_create(int port, int backlog);
int server_destroy(server_t* server);

int server_accept(server_t* server, connection_t* connection);
message_t* server_recv_msg(server_t* server, connection_t* connection);
int server_send_msg(server_t* server, message_t* message, connection_t* connection);
int server_close_connection(server_t* server, connection_t* connection);

#ifdef __cplusplus
}
#endif

#endif
