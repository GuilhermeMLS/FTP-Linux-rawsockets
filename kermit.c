/*
 * Trabalho de Rede de Computadores I - BCC DInf UFPR
 * Prof. Dr. Luiz Carlos Pessoa Albini
 *
 * Guilherme M. Lopes GRR20163043
 * Leonardo Stefan GRR20163052
 *
 * Programa de conexão remota e tranferência de arquivos via Linux RawSocket.
 *
 * */
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

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Error: missing parameter\n");
        printf("Usage:\n For master./kermit <socket> m\n For slave ./kermit <socket> s\n");
        return 1;
    }
    if (*(argv[2]) == 'm') {
        // master
        int socket  = ConexaoRawSocket(argv[1]);
        seq_send    = 0;
        seq_receive = 0;
        char *line;
        while(1) {
            int cmd = readCommand(&line);
            executeCommand(socket, cmd, line);
            printf("\n");
        }
    } else if (*(argv[2]) == 's') {
        // slave
        context     = 0;
        int socket  = ConexaoRawSocket(argv[1]);
        seq_send    = 0;
        seq_receive = 0;
        message msg_receive;
        while(1){
            while(!receiveMessage(socket, &msg_receive));
            executeOrder(socket, msg_receive);
        }
    } else {
        printf("Error: parameter kermit type invalid\n");
        printf("Usage:\n For master./kermit <socket> m\n For slave ./kermit <socket> s\n");

        return 1;
    }

    return 0;
}

