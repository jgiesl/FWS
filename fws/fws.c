
#include <asn_internal.h>

#include <fws.h>
#include <Envelope.h>

#include <server.h>
#include <client.h>
#include <message.h>

#define PEER_CLIENT 1
#define PEER_SERVER 2

fws_t* fws_init()
{
	fws_t* ctx;
	ctx = (fws_t*)FWSALLOC(sizeof(fws_t));
	memset(ctx, 0, sizeof(fws_t));

	return ctx;
}

int fws_uninit(fws_t* ctx)
{
	if(ctx != 0)
		FWSFREE(ctx);

	return RET_OK;
}

int fws_done(fws_t* ctx)
{
	int ret = RET_OK;

	if(ctx->peer_type == PEER_SERVER)
	{
		if(server_destroy(ctx->choice.server) != 0)
			ret = RET_ERR;
	}
	else if(ctx->peer_type == PEER_CLIENT)
	{
		if(client_destroy(ctx->choice.client) != 0)
			ret = RET_ERR;
	}

	return ret;
}

int fws_server_set_func_list(fws_t* ctx, func_list_t* func_list, unsigned long func_count)
{
	ctx->func_count = func_count;
	ctx->func_list = func_list;

	return RET_OK;
}

int fws_server_bind(fws_t* ctx, int port, int backlog /* = 5 */)
{
	ctx->peer_type = PEER_SERVER;
	ctx->choice.server = server_create(port, backlog);
	if(ctx->choice.server == 0)
		return RET_ERR;

	return RET_OK;
}

int fws_server_accept(fws_t* ctx)
{
	int ret = 0;
	if(ctx->peer_type == PEER_SERVER)
	{
		if((server_accept(ctx->choice.server, &ctx->client_conn)) != 0)
			ret = RET_ERR;
	}

	return ret;
}

int fws_server_serve(fws_t* ctx)
{
	int ret = RET_OK;
	char err_code[256] = {0};

	if(ctx->client_conn.socket == 0)
		return RET_ERR;

	if(ctx->peer_type == PEER_SERVER)
	{
		unsigned long msg_in_size = 0;
		unsigned char* msg_in_data = 0;
		unsigned long msg_out_size = 0;
		unsigned char* msg_out_data = 0;

		message_t* message_in;
		message_t* message_out;

		Envelope_t* env_req = 0;
		Envelope_t* env_resp = (Envelope_t*)CALLOC(1, sizeof(Envelope_t));
		memset(env_resp, 0, sizeof(Envelope_t));

		// receive message
		message_in = server_recv_msg(ctx->choice.server, &ctx->client_conn);
		if(message_in == NULL)
		{
			snprintf(err_code, sizeof(err_code), "Error when receiving message");
			ret = RET_ERR;
		}
		else
			ret = RET_OK;

		if(ret == RET_OK)
		{
			asn_dec_rval_t r;

			// decode message into envelope
			msg_in_size = message_get_size(message_in);
			msg_in_data = message_get_stream(message_in);
			r = ber_decode(NULL, &asn_DEF_Envelope, (void **)&env_req, msg_in_data, msg_in_size);
			if(r.consumed <= 0)
			{
				snprintf(err_code, sizeof(err_code), "Error during BER decoding");
				ret = RET_ERR;
			}
		}

		if(ret == RET_OK)
		{
			// if body is occured
			if(env_req->body_or_fault.present == body_or_fault_PR_body)
			{
				int i;
				// decision for processing function
				int func_index = -1;
				for(i=0; i<(int)ctx->func_count; i++)
				{
					if(memcmp(env_req->body_or_fault.choice.body.type.buf, ctx->func_list[i].func_identifier, env_req->body_or_fault.choice.body.type.size) == 0)
					{
						func_index = i;
						break;
					}
				}
				// call processing function
				if(func_index != -1)
				{
					if((ctx->func_list[func_index].func_callback(env_req->body_or_fault.choice.body.data.buf, env_req->body_or_fault.choice.body.data.size, &msg_out_data, &msg_out_size)) != 0)
					{
						snprintf(err_code, sizeof(err_code), "Error during serving request");
						ret = RET_ERR;
					}
				}
				else
				{
					snprintf(err_code, sizeof(err_code), "Serve function has not been found");
					ret = RET_ERR;
				}
			}
			else
			{
				snprintf(err_code, sizeof(err_code), "No body tag in incoming message");
				ret = RET_ERR;
			}
		}

		if(message_in != NULL)
			message_destroy(message_in);

		// create response
		message_out = message_create();
		if(message_out != NULL && ctx->client_conn.socket != 0)
		{
			asn_enc_rval_t r;
			unsigned char* msg_data = 0;
			unsigned long msg_maxsize = 0;

			if(msg_out_data != 0 && ret == RET_OK)
			{
				// body
				env_resp->body_or_fault.present = body_or_fault_PR_body;
				OCTET_STRING_fromBuf(&env_resp->body_or_fault.choice.body.type, (char*)env_req->body_or_fault.choice.body.type.buf, env_req->body_or_fault.choice.body.type.size);
				OCTET_STRING_fromBuf(&env_resp->body_or_fault.choice.body.data, (char*)msg_out_data, msg_out_size);

				msg_maxsize = env_resp->body_or_fault.choice.body.data.size + env_resp->body_or_fault.choice.body.type.size;
			}
			else if(ret == RET_ERR)
			{
				// fault
				env_resp->body_or_fault.present = body_or_fault_PR_fault;
				asn_long2INTEGER(&env_resp->body_or_fault.choice.fault.code, code_receiver);
				env_resp->body_or_fault.choice.fault.reason = OCTET_STRING_new_fromBuf(&asn_DEF_UTF8String, err_code, strlen(err_code));

				msg_maxsize = env_resp->body_or_fault.choice.fault.code.size;
				if(env_resp->body_or_fault.choice.fault.reason != 0)
					msg_maxsize += env_resp->body_or_fault.choice.fault.reason->size;
			}

			// DER encode envelope
			msg_maxsize += 1024; // + space for envelope stuff
			msg_data = (unsigned char*)FWSALLOC(msg_maxsize);
			r = der_encode_to_buffer(&asn_DEF_Envelope, env_resp, msg_data, msg_maxsize);
			if(r.encoded <= 0)
				ret = RET_ERR;
			else
			{
				if((message_set_stream(message_out, msg_data, r.encoded)) != 0)
					ret = RET_ERR;

				// send response
				if((server_send_msg(ctx->choice.server, message_out, &ctx->client_conn)) != 0)
					ret = RET_ERR;

				message_destroy(message_out);
			}

			if(msg_data != 0)
				FWSFREE(msg_data);
			if(msg_out_data != 0)
				FWSFREE(msg_out_data);
		}
		else
			ret = RET_ERR;

		// free memory
		if(env_req != 0)
			asn_DEF_Envelope.free_struct(&asn_DEF_Envelope, env_req, 0);
		if(env_resp != 0)
			asn_DEF_Envelope.free_struct(&asn_DEF_Envelope, env_resp, 0);
	}

	return ret;
}

int fws_client_connect(fws_t* ctx, char* address, int port)
{
	int ret = 0;

	// create client
	ctx->peer_type = PEER_CLIENT;
	ctx->choice.client = client_create(address, port);
	if(ctx->choice.client == 0)
		return -1;

	return ret;
}

int fws_client_call(fws_t* ctx, char* message_type, unsigned char* in_msg, unsigned long in_size, unsigned char** out_msg, unsigned long* out_size)
{
	int ret = RET_OK;
	
	if(ctx->choice.client == 0)
		return RET_ERR;
		
	if(ctx->peer_type == PEER_CLIENT)
	{
		unsigned long msg_maxsize;
		unsigned char* msg_data = 0;

		message_t* message_in;
		message_t* message_out;

		asn_enc_rval_t renc;
		asn_dec_rval_t rdec;
		Envelope_t* env_resp = 0;
		Envelope_t* env_req = 0;

		// envelope construction
		env_req = (Envelope_t*)calloc(1, sizeof(Envelope_t));
		memset(env_req, 0, sizeof(Envelope_t));

		env_req->body_or_fault.present = body_or_fault_PR_body;
		OCTET_STRING_fromString(&env_req->body_or_fault.choice.body.type, message_type);
		OCTET_STRING_fromBuf(&env_req->body_or_fault.choice.body.data, (char*)in_msg, in_size);

		// DER encode envelope
		msg_maxsize = env_req->body_or_fault.choice.body.data.size+1024; // size of body + space for envelope stuff
		msg_data = (unsigned char*)FWSALLOC(msg_maxsize);
		renc = der_encode_to_buffer(&asn_DEF_Envelope, env_req, msg_data, msg_maxsize);
		if(renc.encoded <= 0)
			ret = RET_ERR;

		// send request
		message_in = message_create();
		message_set_stream(message_in, msg_data, renc.encoded);
		if(client_send_msg(ctx->choice.client, message_in) != 0)
			ret = RET_ERR;
		message_destroy(message_in);

		FWSFREE(msg_data);

		// get response
		message_out = client_recv_msg(ctx->choice.client);
		if(message_out != NULL)
		{
			// get stream
			unsigned long stream_size = message_get_size(message_out);
			unsigned char* stream_data = message_get_stream(message_out);

			// BER decoding of Envelope
			rdec = ber_decode(NULL,&asn_DEF_Envelope, (void**)&env_resp, stream_data, stream_size);
			if(env_resp->body_or_fault.present == body_or_fault_PR_body)
			{
				// copy body to output data
				*out_size = env_resp->body_or_fault.choice.body.data.size;
				*out_msg = (unsigned char*)FWSALLOC(*out_size);
				memcpy(*out_msg, env_resp->body_or_fault.choice.body.data.buf, *out_size);
			}
			else if(env_resp->body_or_fault.present == body_or_fault_PR_fault)
			{
				ret = RET_ERR;
			}

			message_destroy(message_out);
		}
		else
			ret = RET_ERR;

		// free memory
		if(env_req != 0)
			asn_DEF_Envelope.free_struct(&asn_DEF_Envelope, env_req, 0);
		if(env_resp != 0)
			asn_DEF_Envelope.free_struct(&asn_DEF_Envelope, env_resp, 0);
	}

	return ret;
}
