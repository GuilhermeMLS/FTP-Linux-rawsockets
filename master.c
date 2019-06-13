#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "inout.h"
#include "messages.h"
#include "commands.h"
#include "connection.h"

unsigned int seq_send;
unsigned int seq_receive;

char context = LOCAL;

int main(int argc, char *argv[]){
    int socket  = ConnectionRawSocket(argv[1]);
    seq_send    = 0;
    seq_receive = 0;
    char *line;

    while(1){
        int cmd = readCommand(&line);
        executeCommand(socket, cmd, line);
    }

    return 0;
}
