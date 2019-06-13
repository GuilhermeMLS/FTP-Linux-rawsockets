#include "messages.h"
unsigned char calculaCRC(mensagem *msg){
	char crc = 0;
	char *msg_char = (char *)msg;
	for(unsigned int i = 1; i < 4+TAM_DADOS-1; i++){
		crc ^= msg_char[i];
	}

	return crc;
}

int checaCRC(mensagem *msg){
	unsigned char crc = calculaCRC(msg);
	#ifdef DEBUG
        printf("%d == %d :: %d\n", crc, msg->crc, (crc == msg->crc));
	#endif
	return (crc == msg->crc);
}