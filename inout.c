#include "inout.h"
#include <errno.h>
#include <sys/statvfs.h>

int checkSpace(long long int size){
    struct statvfs stat;
    statvfs("./", &stat);

    return (size < (stat.f_bavail * stat.f_bsize));
}

char readCommand(char **line){
    char prompt[4] = {'X', '>', '>', '\0'};
    prompt[0] = context;
    char *string = (char *) malloc(sizeof(char)*100);
    *line = readline(prompt);
    sscanf(*line, "%s", string);

    if(strcmp(string, "ls") == 0){
        return CMD_LS;
    } else if(strcmp(string, "cd") == 0){
        return CMD_CD;
    } else if(strcmp(string, "put") == 0){
        return CMD_PUT;
    }else if(strcmp(string, "get") == 0){
        return CMD_GET;
    }else if(strcmp(string, "R") == 0){
        return CMD_REMOTE;
    }else if(strcmp(string, "L") == 0){
        return CMD_LOCAL;
    }else if(strcmp(string, "quit") == 0){
        return CMD_QUIT;
    } else {
        return CMD_UNK;
    }
}

void commandError(){
    printf("command not found\n");
}

void executeCD(int socket, char *line){
    char str1[100];
    char str2[100];
    char str3[100];
    int i = sscanf(line, "%s %s %s", str1, str2, str3);
    if(i == 2){
        if(context == LOCAL){
            if(chdir(str2) == -1){
                switch(errno){
                    case ENOENT:
                        printf("not such directory\n");
                        break;
                    case ENOTDIR:
                        printf("no such directory\n");
                        break;
                    case EACCES:
                        printf("access denied\n");
                        break;
                    default:
                        printf("unknown error\n");
                        break;
                }
            }
        } else if(context == REMOTE){
            masterCD(socket, str2);
        }
    } else {
        commandError();
    }
}

FILE *callLS(int type){
    FILE *fp;
    switch (type) {
        case 0:
            fp = popen("ls", "r");
            break;
        case 1:
            fp = popen("ls -l", "r");
            break;
        case 2:
            fp = popen("ls -a", "r");
            break;
        case 3:
            fp = popen("ls -l -a", "r");
            break;
    }
    return fp;
}

void executeLS(int socket, char *line){
    char str[4][100];
    int type = 0;
    int i = sscanf(line, "%s %s %s %s", str[0], str[1], str[2], str[3]);
    if((i < 4) && (i > 0)){
        for(int k = 0; k < 4; k++){
            if(strcmp(str[k],"-l") == 0){
                type += 1;
            }
            if(strcmp(str[k],"-a") == 0){
                type += 2;
            }
        }
        if(context == LOCAL){
            FILE *fp = callLS(type);

            char c;
            while ((c = fgetc(fp)) != EOF){
                printf("%c", c);
            }
        } else if(context == REMOTE){
            masterLS(socket, type);
        }
    } else {
        commandError();
    }
}

int executeCommand(int socket, char type, char *line){
    char file_name[DATA_SIZE];
    switch(type){
        case CMD_CD:
            executeCD(socket, line);
            break;
        case CMD_LS:
            executeLS(socket, line);
            break;
        case CMD_PUT:
            sscanf(line, "%s %s\n", file_name, file_name);
            masterPUT(socket, file_name);
            break;
        case CMD_GET:
            sscanf(line, "%s %s\n", file_name, file_name);
            masterGET(socket, file_name);
            break;
        case CMD_LOCAL:
            context = LOCAL;
            break;
        case CMD_REMOTE:
            context = REMOTE;
            break;
        case CMD_QUIT:
            seq_send++;
            sendQUIT(socket);
            exit(-1);
        case CMD_UNK:
            commandError();
            break;
    }

    return 0;
}

int executeOrder(int socket, message msg){
    switch(msg.type){
        case TYPE_CD:
            slaceCD(socket, (char *) msg.data);
            break;
        case TYPE_LS:
            slaveLS(socket, msg.data[0]);
            break;
        case TYPE_PUT:
            slavePUT(socket, (char *) msg.data);
            break;
        case TYPE_GET:
            slaveGET(socket, (char *) msg.data);
            break;
        case TYPE_QUIT:
            exit(-1);
    }

    return 0;
}
