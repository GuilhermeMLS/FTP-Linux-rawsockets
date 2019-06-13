#include <stdio.h>
#include <unistd.h>

#include "messages.h"
#include "inout.h"
#include "commands.h"
#include "connection.h"

unsigned int seq_send;
unsigned int seq_receive;

char context = 0;

int main(int argc, char *argv[]){
    int socket  = ConnectionRawSocket(argv[1]);
    seq_send    = 0;
    seq_receive = 0;

    message msg_receive;

    while(1){
        while(!receiveMessage(socket, &msg_receive));
        executeOrder(socket, msg_receive);
    }

    return 0;
}
