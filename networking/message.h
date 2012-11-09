#ifndef __MESSAGE__
#define __MESSAGE__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	unsigned long size;
	unsigned char* stream;
	unsigned long position;
} message_t;

message_t* message_create();
int message_destroy(message_t* message);

unsigned long message_get_size(message_t* message);
unsigned char* message_get_stream(message_t* message);
int message_set_stream(message_t* message, unsigned char* data, unsigned long size);
int message_append_stream(message_t* message, unsigned char* data, unsigned long size);
int message_clear_stream(message_t* message);

int message_write_uchar (message_t* message, const unsigned char data);
int message_write_schar (message_t* message, const signed char data);
int message_write_ushort(message_t* message, const unsigned short data);
int message_write_sshort(message_t* message, const signed short data);
int message_write_ulong (message_t* message, const unsigned long data);
int message_write_slong (message_t* message, const signed long data);
int message_write_float (message_t* message, const float data);
int message_write_double(message_t* message, const double data);
int message_write_string(message_t* message, const unsigned char* data, const unsigned long size);

unsigned char  message_read_uchar (message_t* message);
signed char    message_read_schar (message_t* message);
unsigned short message_read_ushort(message_t* message);
signed short   message_read_sshort(message_t* message);
unsigned long  message_read_ulong (message_t* message);
signed long    message_read_slong (message_t* message);
float          message_read_float (message_t* message);
double         message_read_double(message_t* message);
unsigned long  message_read_string(message_t* message, unsigned char** data);

int message_free_string(unsigned char* data);

#ifdef __cplusplus
}
#endif

#endif
