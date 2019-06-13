#ifndef __COMMANDS
#define __COMMANDS

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "messages.h"

void masterLS(int socket, int args);
void slaveLS(int socket, int args);
void masterCD(int socket, char *dir_name);
void slaveCD(int socket, char *dir_name);
void readFile(int socket, char *file_name);
void sendFile(int socket, char *file_name);
void masterPUT(int socket, char *file_name);
void slavePUT(int socket, char *file_name);
void masterGET(int socket, char *file_name);
void slaveGET(int socket, char *file_name);

#endif
