#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "messages.h"
#include "connection.h"

struct timeval time_to;

extern char context;

void mountMessage(message *msg, unsigned int type, unsigned int size, char *data){
    memset(msg->data, 0, DATA_SIZE);
    msg->type = type;
    // calculado antes, passado como argumento:
    msg->size = size;

    if(size > 0) {
        for(unsigned int i = 0; i < size; i++){
            msg->data[i] = data[i];
        }
    }
}

unsigned char calculateCRC(message *msg){
    // Leo está implementando
}

int checkCRC(message *msg){
    // Leo está implementando
}

// TODO: justificar o timeOut escolhido depois; verificar o funcionamento desta função
int sendMessage(int socket, message *msg){
    message receive;
    msg->init = PREAMBLE;
    msg->seq = seq_send; // atualizar antes, fora do laço de repetição de envio
    msg->crc = calculateCRC(msg);
    time_to.tv_sec = 1;
    time_to.tv_usec = 0;
    int cont_timeout = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &time_to, sizeof(time_to));

    do{
#ifdef DEBUG
        printf("---> SENDING: \n");
        printMessage(msg);
#endif
        send(socket, msg, SIZE_MSG, 0);
        cont_timeout++;

        if(cont_timeout > 99){
            if(context){
                printf("TIMEOUT: master terminated - no slave connection (unknown status)\n");
            } else {
                printf("Connection with master lost - closing connection.\n");
            }

            exit(-1);
        }
    } while(receiveMessage(socket, &receive) != TYPE_ACK);

    time_to.tv_sec = 0;
    time_to.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &time_to, sizeof(time_to));

    return 0;
}

int sendACK(int socket){
    message msg;
    msg.init = PREAMBLE;
    msg.type = TYPE_ACK;
    msg.seq = seq_send;
    msg.crc = calculateCRC(&msg);

#ifdef DEBUG
    printf("---> SENDING: \n");
    printMessage(&msg);
#endif
    send(socket, &msg, SIZE_MSG, 0);

    return 0;
}

int sendNACK(int socket){
    message msg;
    msg.init = PREAMBLE;
    msg.type = TYPE_NACK;
    msg.seq = seq_send;
    msg.crc = calculateCRC(&msg);

#ifdef DEBUG
    printf("---> SENDING: \n");
    printMessage(&msg);
#endif
    send(socket, &msg, SIZE_MSG, 0);

    return 0;
}

int sendOK(int socket){
    message msg;
    message receive;
    msg.init = PREAMBLE;
    msg.type = TYPE_OK;
    msg.seq = seq_send;
    msg.crc = calculateCRC(&msg);

    do{
#ifdef DEBUG
        printf("---> SENDING: \n");
        printMessage(&msg);
#endif
        send(socket, &msg, SIZE_MSG, 0);
    } while(receiveMessage(socket, &receive) != TYPE_ACK);

    return 0;
}

int sendQUIT(int socket){
    message msg;
    message receive;
    msg.init = PREAMBLE;
    msg.type = TYPE_QUIT;
    msg.seq = seq_send;
    msg.crc = calculateCRC(&msg);

    do{
#ifdef DEBUG
        printf("---> SENDING: \n");
        printMessage(&msg);
#endif
        send(socket, &msg, SIZE_MSG, 0);
    } while(receiveMessage(socket, &receive) != TYPE_ACK);

    return 0;
}

int receiveMessage(int socket, message *msg){
    int sequence = -1;
    recv(socket, msg, SIZE_MSG, 0);

    if(msg->init == PREAMBLE){
#ifdef DEBUG
        printf("---> RECEIVING:\n");
    	printMessage(msg);
#endif
        sequence = checkSequence(msg, &seq_receive);
        if(checkCRC(msg)){
            if((sequence == SEQ_OK) || (msg->type == TYPE_ACK)){
                if(msg->type != TYPE_ACK){
                    sendACK(socket);
                }

                return msg->type;
            } else if((sequence == SEQ_REPET) && (msg->type != TYPE_ACK)){
                if(msg->type != TYPE_NACK){
                    sendACK(socket);

                    return SEQ_REPET;
                }
            }
        } else {
            sendNACK(socket);
            seq_receive--;

            return CRC_NOTOK;
        }
    }

    return sequence;
}

void printMessage(message *msg){
    printf("Size: %d\n", msg->size);
    printf("Sequence: %d\n", msg->seq);
    printf("Type: %d\n", msg->type);
    printf("Data:");
    for (int i = 0; i < DATA_SIZE; ++i) {
        printf(" %x", msg->data[i]);
    }
    printf("\n");
    printf("CRC: %d\n", msg->crc);
}

// Checar essa função, acho que não está correta
int checkSequence(message *msg, unsigned int *seq){
    int next_seq = (*seq+1) % 64;
    if(msg->seq == next_seq){
        *seq = next_seq;

        return SEQ_OK;
    } else if (msg->seq == *seq){

        return SEQ_REPET;
    } else {

        return SEQ_NOTOK;
    }
}
