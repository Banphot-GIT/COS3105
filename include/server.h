#ifndef SERVER_H 
#define SERVER_H


#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <netdb.h>
#include "common.h"
#include "protocol.h"
#include "board.h"
#include "task.h"

#define MAX_BOARDS 50
#define MAX_TASKS 200

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#define SERVER_PORT "3030"
#define MESSAGE_SIZE 2000

typedef struct {
    SOCKET clientSocket;
    char *rawInput;
} CommandContext;

// Define function pointer CommandHandler
typedef void (*CommandHandler)(CommandContext *ctx);

typedef struct {
    char *commandName;
    CommandHandler handler;
} CommandEntry;


// Server
void handleAddBoard(CommandContext *ctx);
void handleListBoards(CommandContext *ctx);
void handleUpdateBoard(CommandContext *ctx);
void handleDeleteBoard(CommandContext *ctx);

void handleAddTask(CommandContext *ctx);
void handleListTasks(CommandContext *ctx);
void handleUpdateTask(CommandContext *ctx);
void handleDeleteTask(CommandContext *ctx);


// Common
int handleClient(SOCKET socket_client,char *read);
void handle_shutdown(int sig);
int setup(char *port);
#endif