
#include <fws.h>

#include "TestOperation.h"

// serve request of type TestMessage
RETVAL TestMessageFunc(unsigned char* in_msg, unsigned long in_size, unsigned char** out_msg, unsigned long* out_size)
{
	asn_dec_rval_t rdec;
	asn_enc_rval_t renc;
	unsigned char data_buffer[512] = {0};
	long num;

	TestOperation_t* INPUT = 0;
	TestOperation_t OUTPUT;
	memset(&OUTPUT, 0, sizeof(TestOperation_t));
	OUTPUT.present = TestOperation_PR_output;

	fprintf(stderr,"--------------------\r\n");
	fprintf(stderr,"     TestMessage    \r\n");
	fprintf(stderr,"--------------------\r\n");

	// BER decoding of incoming message
	rdec = ber_decode(NULL,&asn_DEF_TestOperation, (void**)&INPUT, in_msg, in_size);

	// show request
	asn_INTEGER2long(&INPUT->choice.input.number, &num);
	fprintf(stderr,"Text:%s\r\n", INPUT->choice.input.text.buf);
	fprintf(stderr,"Number:%d\r\n", num);

	// set response
	OCTET_STRING_fromString(&OUTPUT.choice.output.result, "Response data");
	asn_long2INTEGER(&OUTPUT.choice.output.code, 0);

	// DER encoding of outgoing message
	renc = der_encode_to_buffer(&asn_DEF_TestOperation, &OUTPUT, data_buffer, sizeof(data_buffer));
	
	// copy message to output data
	*out_size = renc.encoded;
	*out_msg = (unsigned char*)FWSALLOC(*out_size);
	memcpy(*out_msg, data_buffer, *out_size);

	// free memory
	asn_DEF_TestOperation.free_struct(&asn_DEF_TestOperation, INPUT, 0);

	fprintf(stderr,"--------------------\r\n");

	return RET_OK;
}

// main function for server implementation
int main_server()
{
	// init server
	fws_t* fws = fws_server_init();

	// specify function list
	func_list_t func_list[1] = {{"TestMessage",TestMessageFunc}};
	fws_server_set_func_list(fws, func_list, 1);

	// bind socket
	fws_server_bind(fws, 4242, 100);

	// wait for incoming requests
	while(1)
	{
		// accept request
		fws_server_accept(fws);

		// serve request (call user-defined function)
		fws_server_serve(fws);
	}

	// uninit
	fws_server_done(fws);
	fws_server_uninit(fws);

	return 0;
}

// main function for client implementation
int main_client()
{
	unsigned char* buffer = 0;
	asn_enc_rval_t renc;

	unsigned char* out = 0;
	unsigned long size = 0;

	fws_t* fws = 0;

	// create request and fill it with data
	TestOperation_t REQUEST;
	memset(&REQUEST, 0, sizeof(REQUEST));
	REQUEST.present = TestOperation_PR_input;

	asn_long2INTEGER(&REQUEST.choice.input.number, 1);
	OCTET_STRING_fromString(&REQUEST.choice.input.text, "Request data");

	// encode request data
	buffer = (unsigned char*)malloc(1024);
	renc = der_encode_to_buffer(&asn_DEF_TestOperation, &REQUEST, buffer, 1024);

	// init client and connect to server
	fws = fws_client_init();
	if(fws_client_connect(fws, "localhost", 4242) == RET_OK)
	{
		// call TestMessage function and get response
		int ret = fws_client_call(fws, "TestMessage", buffer, renc.encoded, &out, &size);
		if(ret == 0)
		{
			asn_dec_rval_t rdec;
			TestOperation_t* RESPONSE = 0;
			long code;

			// decode response data
			rdec = ber_decode(NULL,&asn_DEF_TestOperation, (void**)&RESPONSE, out, size);
			if(rdec.code == RC_OK)
			{
				// show response
				asn_INTEGER2long(&RESPONSE->choice.output.code, &code);
				fprintf(stderr,"Code:%d\r\n", code);
				fprintf(stderr,"Result:%s\r\n", RESPONSE->choice.output.result.buf);
			}

			// free memory
			asn_DEF_TestOperation.free_struct(&asn_DEF_TestOperation, RESPONSE, 0);
		}
		// free memory
		FWSFREE(out);
	}

	// uninit
	fws_client_done(fws);
	fws_client_uninit(fws);

	// free memory
	asn_DEF_TestOperation.free_struct(&asn_DEF_TestOperation, &REQUEST, 1);
	free(buffer);

	return 0;
}

int main()
{
	int item;

	fprintf(stderr,"---- what do you want to run? ----\r\n");
	fprintf(stderr,"1) server\r\n");
	fprintf(stderr,"2) client\r\n");

	scanf("%d",&item);
	getchar();

	if(item == 1)
		main_server();
	else if(item == 2)
		main_client();
	
	return 0;
}
