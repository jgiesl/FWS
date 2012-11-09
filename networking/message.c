
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "platform.h"

message_t* message_create()
{
	message_t* message = (message_t*)malloc(sizeof(message_t));
	message->size = 0;
	message->stream = NULL;
	message->position = 0;

	return message;
}

int message_destroy(message_t* message)
{
	if(message->stream != NULL)
	{
		free(message->stream);
		message->stream = NULL;
	}

	message->size = 0;
	message->position = 0;

	if(message != NULL)
	{
		free(message);
		message = NULL;
	}

	return 0;
}

unsigned long message_get_size(message_t* message)
{
	return message->size;
}

unsigned char* message_get_stream(message_t* message)
{
	return message->stream;
}

int message_set_stream(message_t* message, unsigned char* data, unsigned long size)
{
	message->stream = (unsigned char*)malloc(size*sizeof(unsigned char));
	memcpy(message->stream, data, size);

	message->size = size;
	message->position = 0;

	return 0;
}

int message_append_stream(message_t* message, unsigned char* data, unsigned long size)
{
	message->stream = (unsigned char*)realloc(message->stream,message->size+(size*sizeof(unsigned char)));
	memcpy(message->stream+message->size, data, size);

	message->size += size;
	message->position = 0;

	return 0;
}

int message_clear_stream(message_t* message)
{
	if(message->stream != NULL)
	{
		free(message->stream);
		message->stream = NULL;
	}

	message->size = 0;
	message->position = 0;

	return 0;
}

int message_write_uchar (message_t* message, const unsigned char data)
{
	message->stream = (unsigned char*)realloc(message->stream,message->size+sizeof(data));
	memcpy(&message->stream[message->position], &data, sizeof(data));
	message->size += sizeof(data);

	message->position = message->size;

	return 0;
}

int message_write_schar (message_t* message, const signed char data)
{
	message->stream = (unsigned char*)realloc(message->stream,message->size+sizeof(data));
	memcpy(&message->stream[message->position], &data, sizeof(data));
	message->size += sizeof(data);

	message->position = message->size;

	return 0;
}

int message_write_ushort(message_t* message, const unsigned short data)
{
	uint16_t network_data = htons(data);
	
	message->stream = (unsigned char*)realloc(message->stream,message->size+sizeof(network_data));
	memcpy(&message->stream[message->position], &network_data, sizeof(network_data));
	message->size += sizeof(network_data);

	message->position = message->size;

	return 0;
}

int message_write_sshort(message_t* message, const signed short data)
{
	int16_t network_data = htons(data);
	
	message->stream = (unsigned char*)realloc(message->stream,message->size+sizeof(network_data));
	memcpy(&message->stream[message->position], &network_data, sizeof(network_data));
	message->size += sizeof(network_data);

	message->position = message->size;

	return 0;
}

int message_write_ulong (message_t* message, const unsigned long data)
{
	uint32_t network_data = htonl(data);
	
	message->stream = (unsigned char*)realloc(message->stream,message->size+sizeof(network_data));
	memcpy(&message->stream[message->position], &network_data, sizeof(network_data));
	message->size += sizeof(network_data);

	message->position = message->size;

	return 0;
}

int message_write_slong (message_t* message, const signed long data)
{
	int32_t network_data = htonl(data);
	
	message->stream = (unsigned char*)realloc(message->stream,message->size+sizeof(network_data));
	memcpy(&message->stream[message->position], &network_data, sizeof(network_data));
	message->size += sizeof(network_data);

	message->position = message->size;

	return 0;
}

int message_write_float (message_t* message, const float data)
{
	message->stream = (unsigned char*)realloc(message->stream,message->size+sizeof(data));
	memcpy(&message->stream[message->position], &data, sizeof(data));
	message->size += sizeof(data);

	message->position = message->size;

	return 0;
}

int message_write_double(message_t* message, const double data)
{
	message->stream = (unsigned char*)realloc(message->stream,message->size+sizeof(data));
	memcpy(&message->stream[message->position], &data, sizeof(data));
	message->size += sizeof(data);

	message->position = message->size;

	return 0;
}

int message_write_string(message_t* message, const unsigned char* data, const unsigned long size)
{
	uint32_t network_size = htonl(size);
	
	message->stream = (unsigned char*)realloc(message->stream,message->size+sizeof(network_size));
	memcpy(&message->stream[message->position], &network_size, sizeof(network_size));
	message->size += sizeof(network_size);
	message->position = message->size;

	message->stream = (unsigned char*)realloc(message->stream,message->size+size);
	memcpy(&message->stream[message->position], data, size);
	message->size += size;

	message->position = message->size;

	return 0;
}

unsigned char message_read_uchar(message_t* message)
{
	unsigned char data = 0;
	memcpy(&data, &message->stream[message->position], sizeof(data));
	message->position += sizeof(data);

	return data;
}

signed char message_read_schar(message_t* message)
{
	signed char data = 0;
	memcpy(&data, &message->stream[message->position], sizeof(data));
	message->position += sizeof(data);

	return data;
}

unsigned short message_read_ushort(message_t* message)
{
	uint16_t network_data = 0;
	
	unsigned short data = 0;
	memcpy(&network_data, &message->stream[message->position], sizeof(network_data));
	message->position += sizeof(network_data);
	
	data = ntohs(network_data);

	return data;
}

signed short message_read_sshort(message_t* message)
{
	int16_t network_data = 0;
	
	signed short data = 0;
	memcpy(&network_data, &message->stream[message->position], sizeof(network_data));
	message->position += sizeof(network_data);
	
	data = ntohs(network_data);

	return data;
}

unsigned long message_read_ulong(message_t* message)
{
	uint32_t network_data = 0;
	
	unsigned long data = 0;
	memcpy(&network_data, &message->stream[message->position], sizeof(network_data));
	message->position += sizeof(network_data);
	
	data = ntohl(network_data);

	return data;
}

signed long message_read_slong(message_t* message)
{
	uint32_t network_data = 0;
	
	signed long data = 0;
	memcpy(&network_data, &message->stream[message->position], sizeof(network_data));
	message->position += sizeof(network_data);
	
	data = ntohl(network_data);

	return data;
}

float message_read_float(message_t* message)
{
	float data = 0;
	memcpy(&data, &message->stream[message->position], sizeof(data));
	message->position += sizeof(data);

	return data;
}

double message_read_double(message_t* message)
{
	double data = 0;
	memcpy(&data, &message->stream[message->position], sizeof(data));
	message->position += sizeof(data);

	return data;
}

unsigned long message_read_string(message_t* message, unsigned char** data)
{
	uint32_t network_size = 0;
	
	unsigned long size = 0;
	memcpy(&network_size, &message->stream[message->position], sizeof(network_size));
	message->position += sizeof(network_size);
	size = ntohl(network_size);

	*data = (unsigned char*)malloc(size+1);
	memcpy(*data, &message->stream[message->position], size);
	memset(*data+size, 0, 1);
	message->position += size;

	return size;
}

int message_free_string(unsigned char* data)
{
	if(data != NULL)
	{
		free(data);
		data = NULL;
	}

	return 0;
}
