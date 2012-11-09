
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "platform.h"

#include "server.h"
#include "message.h"

server_t* server_create(int port, int backlog)
{
	server_t* server = 0;
	SOCKADDR_IN addr;
	int reuse;

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

	server = (server_t*)malloc(sizeof(server_t));
	memset(server, 0, sizeof(server_t));
	server->socket = INVALID_SOCKET;

	// list of pointers to pointers to connections
	server->connections = (vector_t*)malloc(sizeof(vector_t));
	vector_init(server->connections, sizeof(connection_t**), 0, 0);

	// socket creation
	server->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(server->socket == INVALID_SOCKET)
	{
		fprintf(stderr, "error: server socket creation\r\n");
		return NULL;
	}

	FD_ZERO(&server->read_fdset);
	FD_SET(server->socket, &server->read_fdset);

	// info structure
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	// reusage of socket
	reuse = 1;
	setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

	// binding
	if(bind(server->socket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		fprintf(stderr, "error: server socket binding\r\n");
		return NULL;
	}

	// start listening
	if(listen(server->socket, backlog) == SOCKET_ERROR)
	{
		fprintf(stderr,"error: listening\r\n");
		return NULL;
	}

	// server is running
	server->status = 1;

	return server;
}

int server_destroy(server_t* server)
{
	if(server == NULL)
		return -1;

	if(server->connections != NULL)
	{
		int i;
		for(i=0; i<(int)vector_length(server->connections); i++)
		{
			connection_t* conn;
			vector_get(server->connections,i,&conn);
			connection_destroy(conn);
		}
		vector_dispose(server->connections);
		free(server->connections);
	}

	// close server socket
	if(server->socket != INVALID_SOCKET)
	{
		if(closesocket(server->socket) == SOCKET_ERROR)
		{
			fprintf(stderr, "error: closesocket\r\n");
			return -1;
		}
	}

	server->socket = INVALID_SOCKET;
	server->status = 0;

	free(server);
	server = NULL;

#ifdef _WIN32
	// winsock uninitialization
	WSACleanup();
#endif

	return 0;
}

int server_accept(server_t* server, connection_t* connection)
{
	int i, maxfds, rv;
	fd_set active_fdset;

	// exit the loop if server is stopped
	if(server->status == 0)
		return -1;

	maxfds = server->socket;
	for(i=0; i<(int)vector_length(server->connections); i++)
	{
		connection_t* conn;
		vector_get(server->connections,i,&conn);
		if(conn->socket > (unsigned int)maxfds)
			maxfds = conn->socket;
	}
	maxfds += 1;

	active_fdset = server->read_fdset;
	rv = select(maxfds, &active_fdset, NULL, NULL, NULL);
	if(rv == -1)
	{	
		fprintf(stderr, "error: select\r\n");
		return -1;
	}
	else if(rv == 0)
	{
		fprintf(stderr, "error: select timeout\r\n");
		return -1;
	}
	else
	{
		if(FD_ISSET(server->socket, &active_fdset))
		{
			connection_t* conn;
			char* ip;

			// info about client
			SOCKADDR* client_info = (SOCKADDR*)malloc(sizeof(SOCKADDR));
			int client_info_size = sizeof(SOCKADDR);

			// accepting incoming connection on server socket
			SOCKET client_socket = accept(server->socket, client_info, &client_info_size);
			if(client_socket == INVALID_SOCKET)
			{
				fprintf(stderr, "error: accept incoming connection\r\n");
				return -1;
			}

			FD_SET(client_socket, &server->read_fdset);

			connection->info = client_info;
			connection->socket = client_socket;

			conn = connection_create();
			conn->info = client_info;
			conn->socket = client_socket;

			// insert pointer to connection to the connection list
			vector_push(server->connections,&conn);

			ip = inet_ntoa(((SOCKADDR_IN*)client_info)->sin_addr); 
			fprintf(stderr,"client %s at socket %d connected\r\n", ip, client_socket);
		}
		else
		{
			int i;
			for(i=0; i<(int)vector_length(server->connections); i++)
			{
				connection_t* conn;
				vector_get(server->connections,i,&conn);
				if(FD_ISSET(conn->socket, &active_fdset))
				{
					connection->info = conn->info;
					connection->socket = conn->socket;
					break;
				}
			}
		}
	}

	return 0;
}

int server_close_connection(server_t* server, connection_t* connection)
{
	char* ip;
	int i;

	for(i=0; i<(int)vector_length(server->connections); i++)
	{
		connection_t* conn;
		vector_get(server->connections,i,&conn);

		// remove connection from vector if socket is found
		if(conn->socket == connection->socket)
		{
			connection_destroy(conn);
			vector_remove(server->connections,i);
			break;
		}
	}

	FD_CLR(connection->socket,&server->read_fdset);
	
	// close client socket
	if(connection->socket != INVALID_SOCKET)
		closesocket(connection->socket);

	ip = inet_ntoa(((SOCKADDR_IN*)connection->info)->sin_addr); 
	fprintf(stderr,"client %s at socket %d disconnected\r\n", ip, connection->socket);

	memset(connection, 0, sizeof(connection_t));
	
	return 0;
}

message_t* server_recv_msg(server_t* server, connection_t* connection)
{
	// receive message (NULL if socket is closed or error occured)
	message_t* message = connection_recv(connection);
	if(message == NULL)
		server_close_connection(server, connection);

	return message;
}

int server_send_msg(server_t* server, message_t* message, connection_t* connection)
{
	// send message (NULL if socket is closed or error occured)
	connection_send(connection, message);

	return 0;
}
