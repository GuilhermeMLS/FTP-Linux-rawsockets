#include "commands.h"
#include "inout.h"
#include <errno.h>

void readFile(int socket, char *file_name){
    int file = open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0777);
    message msg_receive;
    int endfile = 0;
    do {
        int type = receiveMessage(socket, &msg_receive);
        if(type == TYPE_DATA){
            write(file, msg_receive.data, msg_receive.size);
        } else if(type == TYPE_END){
            endfile = 1;
        }
    } while(!endfile);

    close(file);
}

void readLS(int socket){
    message msg_receive;
    int endfile = 0;
    do {
        int type = receiveMessage(socket, &msg_receive);
        if(type == TYPE_PRINT){
            for (int i = 0; i < msg_receive.size; i++) printf("%c", msg_receive.data[i]);
        } else if(type == TYPE_END){
            endfile = 1;
        }
    } while(!endfile);
}

void sendFile(int socket, char *file_name){
    struct stat data_file;
    long long int total_read = 0;
    int c_read = 0;
    char c;
    char data[DATA_SIZE];
    message msg_send;
    int file = open(file_name, O_RDONLY);
    fstat(file, &data_file);

    while(total_read < data_file.st_size){
        c_read = 0;
        while((total_read < data_file.st_size) && (c_read < DATA_SIZE)){
            read(file, &c, 1);
            data[c_read] = c;
            c_read++;
            total_read++;
        }

        seq_send++;
        mountMessage(&msg_send, TYPE_DATA, c_read, data);
        sendMessage(socket, &msg_send);
    }

    seq_send++;
    mountMessage(&msg_send, TYPE_END, sizeof(data), data);
    sendMessage(socket, &msg_send);
}

void masterPUT(int socket, char *file_name){
    int file = open(file_name, O_RDONLY);

    if(file == -1){
        perror("local file access");

        return;
    }

    message msg_send;
    message msg_receive;
    char data[DATA_SIZE];
    seq_send++;
    mountMessage(&msg_send, TYPE_PUT, strlen(file_name), file_name);
    sendMessage(socket, &msg_send);

    while(receiveMessage(socket, &msg_receive) != TYPE_OK);

    seq_send++;
    struct stat starq;
    fstat(file, &starq);
    close(file);
    sprintf(data, "%ld", starq.st_size);
    mountMessage(&msg_send, TYPE_SIZE, strlen(data), data);
    sendMessage(socket, &msg_send);

    int type;
    do {
        type = receiveMessage(socket, &msg_receive);
    } while((type != TYPE_OK) && (type != TYPE_ERROR));

    if(type == TYPE_ERROR){
        if(msg_receive.data[0] == 3){
            printf("storage not available\n");
        } else {
            printf("unknown error\n");
        }
    } else {
        sendFile(socket, file_name);
    }

}

void slavePUT(int socket, char *file_name){
    message msg_receive;
    message msg_send;
    seq_send++;
    sendOK(socket);

    while(receiveMessage(socket, &msg_receive) != TYPE_SIZE);
    seq_send++;

    unsigned char data[1];
    long long int size;
    sscanf(msg_receive.data, "%lld", &size);
    if(checkSpace(size)){
        sendOK(socket);
        readFile(socket, file_name);
    } else {
        data[0] = 3;
        mountMessage(&msg_send, TYPE_ERROR, 1, (char *) data);
        sendMessage(socket, &msg_send);
    }
}

void masterGET(int socket, char *file_name){
    message msg_receive;
    message msg_send;
    seq_send++;
    mountMessage(&msg_send, TYPE_GET, strlen(file_name), file_name);
    sendMessage(socket, &msg_send);

    int type;
    do {
        type = receiveMessage(socket, &msg_receive);
    } while((type != TYPE_OK) && (type != TYPE_ERROR));

    if(type == TYPE_ERROR){
        if(msg_receive.data[0] == 1){
            printf("file doesn't exist\n");
        } else {
            printf("unknown error\n");
        }

        return;
    }

    while(receiveMessage(socket, &msg_receive) != TYPE_SIZE);

    seq_send++;

    unsigned char data[1];
    long long int size;

    sscanf(msg_receive.data, "%lld", &size);
    if(checkSpace(size)){
        sendOK(socket);
        readFile(socket, file_name);
    } else {
        data[0] = 3;
        mountMessage(&msg_send, TYPE_ERROR, 1, (char *) data);
        sendMessage(socket, &msg_send);
        printf("not enough storage\n");
    }
}

void slaveGET(int socket, char *file_name){
    message msg_receive;
    message msg_send;
    char data[DATA_SIZE];

    int file = open(file_name, O_RDONLY);

    if(file == -1){
        if(errno == ENOENT){
            data[0] = 1;
            mountMessage(&msg_send, TYPE_ERROR, 1, data);
        } else {
            data[0] = 0;
            mountMessage(&msg_send, TYPE_ERROR, 1, data);
        }
        seq_send++;
        sendMessage(socket, &msg_send);

        return;
    }

    seq_send++;
    sendOK(socket);

    seq_send++;
    struct stat starq;
    fstat(file, &starq);
    close(file);

    sprintf(data, "%ld", starq.st_size);
    mountMessage(&msg_send, TYPE_SIZE, strlen(data), data);
    sendMessage(socket, &msg_send);

    int type;
    do {
        type = receiveMessage(socket, &msg_receive);
    } while((type != TYPE_OK) && (type != TYPE_ERROR));

    if(type == TYPE_OK){
        sendFile(socket, file_name);
    }

}

void masterLS(int socket, int args){
    message msg_send;
    seq_send++;
    char data[DATA_SIZE];
    data[0] = 1;
    sprintf(data, "%d", args);
    mountMessage(&msg_send, TYPE_LS, 1, data);
    sendMessage(socket, &msg_send);

    readLS(socket);
}

void slaveLS(int socket, int args){
    int c_read = 0;
    FILE *file = callLS(args - '0');
    char c;
    char data[DATA_SIZE];
    message msg_send;
    c = fgetc(file);

    while(c != EOF){
        c_read = 0;
        while((c_read < DATA_SIZE) && (c != EOF)){
            data[c_read] = c;
            c_read++;
            c = fgetc(file);
        }

        seq_send++;
        mountMessage(&msg_send, TYPE_PRINT, c_read, data);
        sendMessage(socket, &msg_send);
    }

    seq_send++;
    mountMessage(&msg_send, TYPE_END, sizeof(data), data);
    sendMessage(socket, &msg_send);

    fclose(file);
}

void masterCD(int socket, char *dir_name){
    message msg_send;
    message msg_receive;
    seq_send++;
    mountMessage(&msg_send, TYPE_CD, strlen(dir_name), dir_name);
    sendMessage(socket, &msg_send);

    int type;
    do {
        type = receiveMessage(socket, &msg_receive);
    } while((type != TYPE_OK) && (type != TYPE_ERROR));

    if(type == TYPE_ERROR){
        if(msg_receive.data[0] == 1){
            printf("directory doesn't exist\n");
        } else if (msg_receive.data[0] == 2){
            printf("access denied\n");
        } else {
            printf("unknown error\n");
        }
    }
}

void slaveCD(int socket, char *dir_name){
    message msg_send;

    if(chdir(dir_name) == -1){
        seq_send++;
        unsigned char data[1];
        switch(errno){
            case ENOENT:
                data[0] = 1;
                mountMessage(&msg_send, TYPE_ERROR, 1, (char *) data);
                break;
            case ENOTDIR:
                data[0] = 1;
                mountMessage(&msg_send, TYPE_ERROR, 1, (char *) data);
                break;
            case EACCES:
                data[0] = 2;
                mountMessage(&msg_send, TYPE_ERROR, 1, (char *) data);
                break;
        }
        sendMessage(socket, &msg_send);
    } else {
        seq_send++;
        mountMessage(&msg_send, TYPE_OK, 0, (char *) msg_send.data);
        mountMessage(&msg_send, TYPE_OK, 0, (char *) msg_send.data);
        sendMessage(socket, &msg_send);
    }
}
