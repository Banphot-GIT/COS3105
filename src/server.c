#include "server.h"
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

SOCKET socket_listen = -1;
sem_t *sem = NULL;

/* -------------------------------------------------------------
 * BOARD HANDLERS
 * ------------------------------------------------------------- */

void handleAddBoard(CommandContext *ctx)
{
    char *boardTitle = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char response[MESSAGE_SIZE];

    if (isEmptyOrSpace(boardTitle)) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_BOARD, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    sem_wait(sem);
    int result = add_board(boardTitle);
    sem_post(sem);

    if (result == 0) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_BOARD, COMMAND_SEPARATOR, STATUS_OK);
    } else {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_BOARD, COMMAND_SEPARATOR, STATUS_FAIL);
    }
    send(ctx->clientSocket, response, strlen(response), 0);
}

void handleListBoards(CommandContext *ctx)
{
    char response[MESSAGE_SIZE];
    FILE *fp = fopen(board_file, "r");
    if (fp == NULL) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LIST_BOARDS, COMMAND_SEPARATOR, STATUS_EMPTY_BOARD);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    char line[256];
    char items[1800] = "";
    int count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        replace_char(line, '\r', '\0');
        replace_char(line, '\n', '\0');
        if (isEmptyOrSpace(line)) continue;

        char *saveptr;
        char *id_str = strtok_r(line, ",", &saveptr);
        char *name = strtok_r(NULL, ",", &saveptr);

        if (id_str && name) {
            char entry[256];
            snprintf(entry, sizeof(entry), "%s%s:%s", (count > 0) ? ";" : "", id_str, name);
            strncat(items, entry, sizeof(items) - strlen(items) - 1);
            count++;
        }
    }
    fclose(fp);

    if (count == 0) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LIST_BOARDS, COMMAND_SEPARATOR, STATUS_EMPTY_BOARD);
    } else {
        snprintf(response, sizeof(response), "%s%s%d%s%s\n", COMMAND_LIST_BOARDS, COMMAND_SEPARATOR, STATUS_OK, COMMAND_SEPARATOR, items);
    }
    send(ctx->clientSocket, response, strlen(response), 0);
}

void handleUpdateBoard(CommandContext *ctx)
{
    char *boardIdStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char *newTitle = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char response[MESSAGE_SIZE];

    if (boardIdStr == NULL || newTitle == NULL || isEmptyOrSpace(boardIdStr) || isEmptyOrSpace(newTitle) || !isNumeric(boardIdStr)) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_BOARD, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    int board_id = atoi(boardIdStr);

    sem_wait(sem);
    int result = update_board(board_id, newTitle);
    sem_post(sem);

    if (result == 0) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_BOARD, COMMAND_SEPARATOR, STATUS_OK);
    } else {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_BOARD, COMMAND_SEPARATOR, STATUS_BOARD_NOT_FOUND);
    }
    send(ctx->clientSocket, response, strlen(response), 0);
}

void handleDeleteBoard(CommandContext *ctx)
{
    char *boardIdStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char response[MESSAGE_SIZE];

    if (boardIdStr == NULL || isEmptyOrSpace(boardIdStr) || !isNumeric(boardIdStr)) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_DELETE_BOARD, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    int board_id = atoi(boardIdStr);

    sem_wait(sem);
    int result = del_board(board_id);
    sem_post(sem);

    if (result == 0) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_DELETE_BOARD, COMMAND_SEPARATOR, STATUS_OK);
    } else {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_DELETE_BOARD, COMMAND_SEPARATOR, STATUS_BOARD_NOT_FOUND);
    }
    send(ctx->clientSocket, response, strlen(response), 0);
}

/* -------------------------------------------------------------
 * TASK HANDLERS
 * ------------------------------------------------------------- */

void handleAddTask(CommandContext *ctx)
{
    char *boardIdStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char *taskTitle = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char response[MESSAGE_SIZE];

    if (boardIdStr == NULL || taskTitle == NULL ||
        isEmptyOrSpace(boardIdStr) || isEmptyOrSpace(taskTitle) || !isNumeric(boardIdStr)) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_TASK, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    int board_id = atoi(boardIdStr);

    sem_wait(sem);
    int result = add_task(board_id, taskTitle);
    sem_post(sem);

    if (result == 0) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_TASK, COMMAND_SEPARATOR, STATUS_OK);
    } else {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_ADD_TASK, COMMAND_SEPARATOR, STATUS_FAIL);
    }
    send(ctx->clientSocket, response, strlen(response), 0);
}

void handleListTasks(CommandContext *ctx)
{
    char *boardIdStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char response[MESSAGE_SIZE];

    if (boardIdStr == NULL || isEmptyOrSpace(boardIdStr) || !isNumeric(boardIdStr)) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LIST_TASKS, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    int target_board_id = atoi(boardIdStr);
    FILE *fp = fopen(task_file, "r");
    if (fp == NULL) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LIST_TASKS, COMMAND_SEPARATOR, STATUS_EMPTY_TASK);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    char line[256];
    char items[1800] = "";
    int count = 0;

    /* อ่านไฟล์ tasks.txt format: task_id,board_id,title */
    while (fgets(line, sizeof(line), fp) != NULL) {
        replace_char(line, '\r', '\0');
        replace_char(line, '\n', '\0');
        if (isEmptyOrSpace(line)) continue;

        char *saveptr;
        char *task_id = strtok_r(line, ",", &saveptr);
        char *b_id = strtok_r(NULL, ",", &saveptr);
        char *title = strtok_r(NULL, ",", &saveptr);

        if (task_id && b_id && title && atoi(b_id) == target_board_id) {
            char entry[256];
            snprintf(entry, sizeof(entry), "%s%s:%s", (count > 0) ? ";" : "", task_id, title);
            strncat(items, entry, sizeof(items) - strlen(items) - 1);
            count++;
        }
    }
    fclose(fp);

    if (count == 0) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_LIST_TASKS, COMMAND_SEPARATOR, STATUS_EMPTY_TASK);
    } else {
        snprintf(response, sizeof(response), "%s%s%d%s%s\n", COMMAND_LIST_TASKS, COMMAND_SEPARATOR, STATUS_OK, COMMAND_SEPARATOR, items);
    }
    send(ctx->clientSocket, response, strlen(response), 0);
}

void handleUpdateTask(CommandContext *ctx)
{
    char *boardIdStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char *taskIdStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char *newTitle = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char response[MESSAGE_SIZE];

    if (boardIdStr == NULL || taskIdStr == NULL || newTitle == NULL ||
        isEmptyOrSpace(boardIdStr) || isEmptyOrSpace(taskIdStr) || isEmptyOrSpace(newTitle) ||
        !isNumeric(boardIdStr) || !isNumeric(taskIdStr)) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_TASK, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    int board_id = atoi(boardIdStr);
    int task_id = atoi(taskIdStr);

    sem_wait(sem);
    int result = update_task(board_id, task_id, newTitle);
    sem_post(sem);

    if (result == 0) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_TASK, COMMAND_SEPARATOR, STATUS_OK);
    } else {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UPDATE_TASK, COMMAND_SEPARATOR, STATUS_TASK_NOT_FOUND);
    }
    send(ctx->clientSocket, response, strlen(response), 0);
}

void handleDeleteTask(CommandContext *ctx)
{
    char *boardIdStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char *taskIdStr = strtok_r(NULL, COMMAND_SEPARATOR, &ctx->rawInput);
    char response[MESSAGE_SIZE];

    if (boardIdStr == NULL || taskIdStr == NULL ||
        isEmptyOrSpace(boardIdStr) || isEmptyOrSpace(taskIdStr) ||
        !isNumeric(boardIdStr) || !isNumeric(taskIdStr)) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_DELETE_TASK, COMMAND_SEPARATOR, STATUS_INVALID_ARGUMENTS);
        send(ctx->clientSocket, response, strlen(response), 0);
        return;
    }

    int board_id = atoi(boardIdStr);
    int task_id = atoi(taskIdStr);

    sem_wait(sem);
    int result = del_task(board_id, task_id);
    sem_post(sem);

    if (result == 0) {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_DELETE_TASK, COMMAND_SEPARATOR, STATUS_OK);
    } else {
        snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_DELETE_TASK, COMMAND_SEPARATOR, STATUS_TASK_NOT_FOUND);
    }
    send(ctx->clientSocket, response, strlen(response), 0);
}

/* -------------------------------------------------------------
 * COMMAND ROUTER
 * ------------------------------------------------------------- */

int handleClient(SOCKET socket_client, char *read)
{
    char response[MESSAGE_SIZE];
    char *saveptr;
    char *commandName = strtok_r(read, ",\n\r", &saveptr);

    if (commandName == NULL)
        return -1;

    CommandContext ctx = {
        .clientSocket = socket_client,
        .rawInput = saveptr,
    };

    CommandEntry commandTable[] = {
        {COMMAND_ADD_BOARD, handleAddBoard},
        {COMMAND_LIST_BOARDS, handleListBoards},
        {COMMAND_UPDATE_BOARD, handleUpdateBoard},
        {COMMAND_DELETE_BOARD, handleDeleteBoard},
        {COMMAND_ADD_TASK, handleAddTask},
        {COMMAND_LIST_TASKS, handleListTasks},
        {COMMAND_UPDATE_TASK, handleUpdateTask},
        {COMMAND_DELETE_TASK, handleDeleteTask},
        {NULL, NULL}
    };

    for (int i = 0; commandTable[i].commandName != NULL; i++) {
        if (strcmp(commandName, commandTable[i].commandName) == 0) {
            printf("Found command: %s\n", commandName);
            commandTable[i].handler(&ctx);
            return 0;
        }
    }

    snprintf(response, sizeof(response), "%s%s%d\n", COMMAND_UNKNOWN, COMMAND_SEPARATOR, STATUS_UNKNOWN);
    send(socket_client, response, strlen(response), 0);
    return -1;
}

void handle_shutdown(int sig)
{
    if (sig == SIGINT) {
        printf("\nCaught Ctrl+C! Cleaning up...\n");
    } else if (sig == SIGTERM) {
        printf("\nTermination request received...\n");
    }

    signal(SIGINT, SIG_IGN);

    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);
    socket_listen = -1;

    printf("Cleaning up semaphore...\n");
    if (sem != NULL) {
        sem_close(sem);
        sem_unlink("/store_lock");
    }

    kill(0, SIGTERM);
    exit(0);
}

int setup(char *port)
{
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    sem = sem_open("/store_lock", O_CREAT, 0644, 1);

    /* เรียก Init ไฟล์จาก Model ของเรา */
    init_board();
    init_task();

    struct addrinfo *bind_address;
    getaddrinfo(0, port, &hints, &bind_address);

    printf("Creating socket...\n");
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    int yes = 1;
    if (setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        fprintf(stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address (Port: %s)...\n", port);
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address);

    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Waiting for connections...\n");
    while (1) {
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);
        SOCKET socket_client = accept(socket_listen, (struct sockaddr *)&client_address, &client_len);
        if (!ISVALIDSOCKET(socket_client)) {
            if (socket_listen == -1) return 0;
            fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        char address_buffer[100];
        getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer,
                    sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
        printf("New connection from %s\n", address_buffer);

        int pid = fork();
        if (pid == 0) {
            CLOSESOCKET(socket_listen);
            while (1) {
                char read[1024];
                int bytes_received = recv(socket_client, read, sizeof(read) - 1, 0);
                if (bytes_received < 1) {
                    printf("Client disconnected.\n");
                    CLOSESOCKET(socket_client);
                    exit(0);
                }
                read[bytes_received] = '\0';
                replace_char(read, '\n', '\0');
                handleClient(socket_client, read);
            }
        }
        CLOSESOCKET(socket_client);
    }
    return 0;
}

int main()
{
    signal(SIGINT, handle_shutdown);
    signal(SIGTERM, handle_shutdown);
    setup(SERVER_PORT);
    return 0;
}