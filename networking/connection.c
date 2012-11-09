
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

#include "connection.h"

connection_t* connection_create()
{
	connection_t* connection = (connection_t*)malloc(sizeof(connection_t));
	connection->socket = INVALID_SOCKET;
	connection->info = NULL;

	return connection;
}

void connection_destroy_callback(void* connection_)
{
	connection_t* connection = (connection_t*)connection_;
	connection_destroy(connection);
}

int connection_destroy(connection_t* connection)
{
	if(connection != NULL)
	{
		connection->socket = INVALID_SOCKET;

		if(connection->info != NULL)
		{
			free(connection->info);
			connection->info = NULL;
		}

		free(connection);
		connection = NULL; 
	}

	return 0;
}

message_t* connection_recv(connection_t* connection)
{
	int bytes_recv;
	uint32_t buffer_size;
	char* buffer;

	// receive buffer size
	uint32_t network_size;
	bytes_recv = recv(connection->socket, (char*)&network_size, sizeof(network_size), 0);
	if(bytes_recv > 0)
	{
	}
	else if(bytes_recv == SOCKET_ERROR)
	{
		fprintf(stderr, "error: recv\r\n");
		return NULL;
	}
	else
	{
		fprintf(stderr, "peer dropped connection\r\n");
		return NULL;
	}

	buffer_size = ntohl(network_size);

	// receive message stream
	buffer = (char*)malloc(buffer_size*sizeof(unsigned char));
	bytes_recv = recv(connection->socket, buffer, buffer_size, 0);

	if(bytes_recv > 0)
	{
		// incoming message construction
		message_t* message = message_create();
		message_set_stream(message,(unsigned char*)buffer,buffer_size);

		free(buffer);
		return message;
	}
	else if(bytes_recv == SOCKET_ERROR)
		fprintf(stderr, "error: recv socket\r\n");
	else
		fprintf(stderr, "peer dropped connection\r\n");

	free(buffer);
	return NULL;
}

int connection_send(connection_t* connection, message_t* message)
{
	int bytes_send;
	unsigned char* buffer;
	uint32_t buffer_size;
	uint32_t network_size;
	
	buffer_size = message_get_size(message);
	buffer = message_get_stream(message);

	// send size of message
	network_size = htonl(buffer_size);
	bytes_send = send(connection->socket, (const char*)&network_size, sizeof(network_size), 0);	

	if(bytes_send > 0)
	{
	}
	else if(bytes_send == SOCKET_ERROR)
	{
		fprintf(stderr, "error: send\r\n");
		return -1;
	}
	else
	{
		fprintf(stderr, "peer dropped connection\r\n");
		return -1;
	}

	// send message stream
	bytes_send = send(connection->socket, (const char*)buffer, buffer_size, 0);

	if(bytes_send > 0)
	{
	}
	else if(bytes_send == SOCKET_ERROR)
	{
		fprintf(stderr, "error: send\r\n");
		return -1;
	}
	else
	{
		fprintf(stderr, "peer unexpectedly dropped connection\r\n");
		return -1;
	}

	return 0;
}
