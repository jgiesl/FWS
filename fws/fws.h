
#ifndef	_FWS_H_
#define	_FWS_H_

#include <asn_application.h>

#include <server.h>
#include <client.h>
#include <connection.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FWSALLOC malloc
#define FWSFREE(x) if(x!=0) free(x); x=0;

#define RETVAL int
#define RET_OK   0
#define RET_ERR -1

typedef RETVAL (*ServeFuncCallback)(unsigned char* in_msg, unsigned long in_size, unsigned char** out_msg, unsigned long* out_size);

typedef struct 
{
	char* func_identifier;
	ServeFuncCallback func_callback;
} func_list_t;

typedef struct
{
	int peer_type;
	union
	{
		client_t* client;
		server_t* server;
	} choice;

	unsigned long func_count;
	func_list_t* func_list;

	connection_t client_conn;
} fws_t;

#define fws_server_init fws_init
#define fws_client_init fws_init

#define fws_server_uninit fws_uninit
#define fws_client_uninit fws_uninit

#define fws_server_done fws_done
#define fws_client_done fws_done

fws_t* fws_init();
int fws_uninit(fws_t* ctx);
int fws_done(fws_t* ctx);

int fws_server_set_func_list(fws_t* ctx, func_list_t* func_list, unsigned long func_count);
int fws_server_bind(fws_t* ctx, int port, int backlog);
int fws_server_accept(fws_t* ctx);
int fws_server_serve(fws_t* ctx);

int fws_client_connect(fws_t* ctx, char* address, int port);
int fws_client_call(fws_t* ctx, char* message_type, unsigned char* in_msg, unsigned long in_size, unsigned char** out_msg, unsigned long* out_size);

#ifdef __cplusplus
}
#endif

#endif	/* _FWS_H_ */
