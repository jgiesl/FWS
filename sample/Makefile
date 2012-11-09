ROOT_PATH=..
LIBS=-lm
CFLAGS=-I. -I $(ROOT_PATH)/asn1c/ -I $(ROOT_PATH)/networking/ -I $(ROOT_PATH)/fws/

ASN1C_SRC=$(wildcard $(ROOT_PATH)/asn1c/*.c)
ASN1C_OBJS=$(ASN1C_SRC:.c=.o)

NETWORK_SRC=$(wildcard $(ROOT_PATH)/networking/*.c)
NETWORK_OBJS=$(NETWORK_SRC:.c=.o)

FWS_SRC=$(wildcard $(ROOT_PATH)/fws/*.c)
FWS_OBJS=$(FWS_SRC:.c=.o)

SAMPLE_SRC=$(wildcard $(ROOT_PATH)/sample/*.c)
SAMPLE_OBJS=$(SAMPLE_SRC:.c=.o)

sample: $(ASN1C_OBJS) $(NETWORK_OBJS) $(FWS_OBJS) $(SAMPLE_OBJS)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(ROOT_PATH)/asn1c/*.o
	rm -f $(ROOT_PATH)/networking/*.o
	rm -f $(ROOT_PATH)/fws/*.o
	rm -f $(ROOT_PATH)/sample/*.o
	rm -f $(ROOT_PATH)/sample/sample