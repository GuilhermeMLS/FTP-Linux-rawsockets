#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "messages.h"
#include "conexaoRawSocket.h"

struct timeval time_to;

extern char context;

void mountMessage(
    message *msg,
    unsigned int type,
    unsigned int size,
    char *data
) {
    memset(msg->data, 0, DATA_SIZE);
    msg->type = type;
    msg->size = size;
    if (size > 0) {
        for (unsigned int i = 0; i < size; i++) {
            msg->data[i] = data[i];
        }
    }
}

#define WIDTH (8 * sizeof(crc_t))
#define TOPBIT (1 << (WIDTH - 1))
crc_t crcTable[256];
void crcInit(void) {
    crc_t remainder = 0;
    for (int dividend = 0; dividend < 256; ++dividend) {
        remainder = dividend << (WIDTH - 8);
        for (crc_t bit = 8; bit > 0; --bit) {
            if (remainder & TOPBIT) {
                remainder = (remainder << 1) ^ CRC_POL;
            } else {
                remainder = (remainder << 1);
            }
        }
        crcTable[dividend] = remainder;
    }
}

unsigned char calculateCRC(message *msg) {
    char *msg_calc = (char *)msg;
    static int init = 0;
    if (!init) {
        #ifdef DEBUG
        printf ("\x1B[33mINICIALIZANDO CRC\x1B[0m");
        #endif
        crcInit();
        init = 1;
    }
    crcInit();
    crc_t data;
    crc_t remainder = 0;
    for (int byte = 1; byte < (DATA_SIZE); ++byte) {
        data = msg_calc[byte] ^ (remainder >> (WIDTH - 8));
        remainder = crcTable[data] ^ (remainder << 8);
    }

    return (remainder);
}

int checkCRC(message *msg) {
    char crc = msg->crc;
    char rCrc = calculateCRC(msg);
    #ifdef DEBUG
    if ((crc != rCrc)) {
        printf("\x1B[31mCRC Error\x1B[0m: received=%d, calculated =%d\n", crc, rCrc);
    }
    #endif

    return (crc == rCrc);
}

int sendMessage(int socket, message *msg) {
    message receive;
    msg->init = PREAMBLE;
    msg->seq = seq_send;
    msg->crc = calculateCRC(msg);
    time_to.tv_sec = 1;
    time_to.tv_usec = 0;
    int cont_timeout = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &time_to, sizeof(time_to));
    do {
        #ifdef DEBUG
        printf("---> SENDING: \n");
        printMessage(msg);
        #endif
        send(socket, msg, SIZE_MSG, 0);
        cont_timeout++;
        if (cont_timeout > TIME_OUT) {
            if (context) {
                printf("TIMEOUT: master terminated => no slave connection (unknown status)\n");
            } else {
                printf("Connection with the master lost => closing connection\n");
            }
            exit(-1);
        }
    } while (receiveMessage(socket, &receive) != TYPE_ACK);
    time_to.tv_sec  = 0;
    time_to.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &time_to, sizeof(time_to));

    return 0;
}

int sendACK(int socket) {
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

int sendNACK(int socket) {
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

int sendOK(int socket) {
    message msg;
    message receive;
    msg.init = PREAMBLE;
    msg.type = TYPE_OK;
    msg.seq = seq_send;
    msg.crc = calculateCRC(&msg);
    do {
        #ifdef DEBUG
        printf("---> SENDING: \n");
        printMessage(&msg);
        #endif
        delay(DELAY);
        send(socket, &msg, SIZE_MSG, 0);
    } while (receiveMessage(socket, &receive) != TYPE_ACK);

    return 0;
}

int sendQUIT(int socket) {
    message msg;
    message receive;
    msg.init = PREAMBLE;
    msg.type = TYPE_QUIT;
    msg.seq = seq_send;
    msg.crc = calculateCRC(&msg);
    do {
        #ifdef DEBUG
        printf("---> SENDING: \n");
        printMessage(&msg);
        #endif
        send(socket, &msg, SIZE_MSG, 0);
    } while (receiveMessage(socket, &receive) != TYPE_ACK);

    return 0;
}

int receiveMessage(int socket, message *msg) {
    int sequence = -1;
    recv(socket, msg, SIZE_MSG, 0);
    if (msg->init == PREAMBLE) {
        #ifdef DEBUG
        printf("---> RECEIVING:\n");
        printMessage(msg);
        #endif
        sequence = checkSequence(msg, &seq_receive);
        if (checkCRC(msg)) {
            if ((sequence == SEQ_OK) || (msg->type == TYPE_ACK)) {
                if (msg->type != TYPE_ACK) {
                    sendACK(socket);
                }

                return msg->type;
            } else if ((sequence == SEQ_REPET) && (msg->type != TYPE_ACK)) {
                if (msg->type != TYPE_NACK) {
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

void printMessage(message *msg) {
    printf("Size: %d\n", msg->size);
    printf("Sequence: %d\n", msg->seq);
    printf("Type: %d\n", msg->type);
    printf("Data:");
    for (int i = 0; i < DATA_SIZE; ++i) {
        printf(" %x", msg->data[i]);
    }
    printf("\n");
    printf("CRC: \x1B[35m%d\n\x1B[0m", msg->crc);
}

int checkSequence(message *msg, unsigned int *seq) {
    int next_seq = (*seq + 1) % 64;
    if (msg->seq == next_seq) {
        *seq = next_seq;

        return SEQ_OK;
    } else if (msg->seq == *seq) {

        return SEQ_REPET;
    } else {

        return SEQ_NOTOK;
    }
}

void delay(int number_of_milliseconds) {
    // Stroing start time
    clock_t start_time = clock();
    // looping till required time is not acheived 
    while (clock() < start_time + number_of_milliseconds);
}
