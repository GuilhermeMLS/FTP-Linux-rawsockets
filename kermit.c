#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "inout.h"
#include "messages.h"
#include "commands.h"
#include "conexaoRawSocket.h"

unsigned int seq_send;
unsigned int seq_receive;

char context = LOCAL;

int socket_device;
int main(int argc, char *argv[])
{
    socket_device = RawSocketConnection(argv[1]);

    if (argc > 2)
    {

        char mode = *(argv[2]);
        if (mode == 'S' || mode == 's')
            context = 0;
        return slave(socket_device);
    }
    else
        return master(socket_device);
}

int master(int socket)
{

    seq_send = 0;
    seq_receive = 0;
    char *line;

    while (1)
    {
        int cmd = readCommand(&line);
        executeCommand(socket, cmd, line);
        printf("\n");
    }

    return 0;
}

int slave(int socket)
{
    seq_send = 0;
    seq_receive = 0;

    message msg_receive;

    while (1)
    {
        while (!receiveMessage(socket, &msg_receive))

            executeOrder(socket, msg_receive);
    }

    return 0;
}
