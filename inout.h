
#ifndef __INOUT
#define  __INOUT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "commands.h"
#include "messages.h"

#define REMOTE 'R'
#define LOCAL 'L'

extern char context;

#define CMD_QUIT 10
#define CMD_REMOTE 20
#define CMD_LOCAL 21
#define CMD_LS 40
#define CMD_CD 41
#define CMD_PUT 42
#define CMD_GET 43
#define CMD_UNK 50

int checkSpace(long long int size);

void commandError();

void executeCD(int socket, char *line);

void executeLS(int socket, char *line);

char readCommand(char **line);

int executeOrder(int socket, message msg);

FILE *callLS(int type);

int executeCommand(int socket, char type, char *line);

#endif
