
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"

#include "client.h"
#include "message.h"

client_t* client_create(char* address, int port)
{
	client_t* client = 0;
	SOCKADDR_IN* info;
	struct hostent* host;
	char* ip;

#ifdef _WIN32
	// winsock initialization
	WORD version = MAKEWORD(2,2);
	WSADATA data;
	if (WSAStartup(version, &data) != 0)
	{
		fprintf(stderr, "error: winsock initialization\r\n");
		return NULL;
	}
#endif

	client = (client_t*)malloc(sizeof(client_t));
	client->connection = connection_create();

	// get host by name
	if((host = gethostbyname(address)) == NULL)
	{
		fprintf(stderr, "error: gethostbyname\r\n");
		return NULL;
	}

	// socket creation
	client->connection->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(client->connection->socket == INVALID_SOCKET)
	{
		fprintf(stderr, "error: server socket creation\r\n");
		return NULL;
	}

	// host info
	info = (SOCKADDR_IN*)malloc(sizeof(SOCKADDR_IN));
	info->sin_family = AF_INET;
	info->sin_port = htons(port);
	memcpy(&(info->sin_addr), host->h_addr, host->h_length);

	client->connection->info = (SOCKADDR*)info;

	// socket connection
	if(connect(client->connection->socket,(SOCKADDR*)client->connection->info,sizeof(*client->connection->info)) == SOCKET_ERROR)
	{
		fprintf(stderr,"error: connecting\r\n");
		return NULL;
	}

	// get info about client
	ip = inet_ntoa(((SOCKADDR_IN*)client->connection->info)->sin_addr); 
	fprintf(stderr,"connection to server %s at socket %d opened\r\n", ip, client->connection->socket);

	// client is running
	client->status = 1;

	return client;
}

int client_destroy(client_t* client)
{
	if(client == NULL)
		return -1;

	// close client socket
	if(client->connection->socket != INVALID_SOCKET)
	{
		char* ip;
		
		ip = inet_ntoa(((SOCKADDR_IN*)client->connection->info)->sin_addr);
		fprintf(stderr,"connection to server %s at socket %d closed\r\n", ip, client->connection->socket);

		if(closesocket(client->connection->socket) == SOCKET_ERROR)
		{
			fprintf(stderr, "error: closesocket\r\n");
			return -1;
		}
	}

	client->connection->socket = INVALID_SOCKET;
	client->status = 0;

	if(client->connection != NULL)
	{
		connection_destroy(client->connection);
		client->connection = NULL;
	}

	free(client);
	client = NULL;
	
#ifdef _WIN32
	// winsock uninitialization
	WSACleanup();
#endif

	return 0;
}

message_t* client_recv_msg(client_t* client)
{
	// receive message (NULL if socket is closed or error occured)
	message_t* message = connection_recv(client->connection);

	return message;
}

int client_send_msg(client_t* client, message_t* message)
{
	// send message (NULL if socket is closed or error occured)
	connection_send(client->connection, message);

	return 0;
}
