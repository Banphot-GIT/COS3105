#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "common.h"
#include "protocol.h"

#define SERVER_IP "127.0.0.1"
#define BUF_SIZE 2048
#define SERVER_PORT "3030"

// data send
void send_cmd(int sock, const char *msg, char *res) {
    char send_buf[BUF_SIZE];
    snprintf(send_buf, sizeof(send_buf), "%s\n", msg);
    send(sock, send_buf, strlen(send_buf), 0);

    memset(res, 0, BUF_SIZE);
    recv(sock, res, BUF_SIZE - 1, 0);
    replace_char(res, '\r', '\0');
    replace_char(res, '\n', '\0');
}

//Task 
void task_menu(int sock, int board_id) {
    int choice;
    char req[BUF_SIZE], res[BUF_SIZE];
    char text[256];
    int task_id;

    while (1) {
        printf("\n========================================\n");
        printf("List Task in Board no. %d\n", board_id);

        snprintf(req, sizeof(req), "%s%s%d", COMMAND_LIST_TASKS, COMMAND_SEPARATOR, board_id);
        send_cmd(sock, req, res);
        printf("%s\n", res);

        printf("What do you want to do?\n");
        printf("1. Create Task\n");
        printf("2. Edit Task\n");
        printf("3. Delete Task\n");
        printf("4. Back to Board Menu\n");
        printf("You select option: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');  
            continue;
        }

        if (choice == 1) {
            printf("Create Task in Board no. %d\n", board_id);
            printf("Please input task title: ");
            scanf(" %[^\n]", text);

            if (isEmptyOrSpace(text)) {
                printf("Error: Title cannot be empty.\n");
                continue;
            }

            snprintf(req, sizeof(req), "%s%s%d%s%s", 
                     COMMAND_ADD_TASK, COMMAND_SEPARATOR, board_id, COMMAND_SEPARATOR, text);
            send_cmd(sock, req, res);
            printf("Response: %s\n", res);

        } else if (choice == 2) {
            printf("Please input task id to edit: ");
            scanf("%d", &task_id);
            printf("Please input new task title: ");
            scanf(" %[^\n]", text);

            if (isEmptyOrSpace(text)) {
                printf("Error: Title cannot be empty.\n");
                continue;
            }

            snprintf(req, sizeof(req), "%s%s%d%s%d%s%s", 
                     COMMAND_UPDATE_TASK, COMMAND_SEPARATOR, board_id, COMMAND_SEPARATOR, task_id, COMMAND_SEPARATOR, text);
            send_cmd(sock, req, res);
            printf("Response: %s\n", res);

        } else if (choice == 3) {
            printf("Please input task id to delete: ");
            scanf("%d", &task_id);

            snprintf(req, sizeof(req), "%s%s%d%s%d", 
                     COMMAND_DELETE_TASK, COMMAND_SEPARATOR, board_id, COMMAND_SEPARATOR, task_id);
            send_cmd(sock, req, res);
            printf("Response: %s\n", res);

        } else if (choice == 4) {
            break;
        }
    }
}

//Board 
int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    int choice;
    char req[BUF_SIZE], res[BUF_SIZE];
    char text[256];
    int board_id;
    int port = atoi(SERVER_PORT);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection to server failed!\n");
        return 1;
    }
    printf("Connected to server successfully (Port %d)!\n", port);

    while (1) {
        printf("\n========================================\n");
        printf("# Board\n");
        printf("1. List Board\n");
        printf("2. Create Board\n");
        printf("3. Update Board\n");
        printf("4. Delete Board\n");
        printf("5. Exit\n");
        printf("Please select option: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }

        if (choice == 1) {
            snprintf(req, sizeof(req), "%s", COMMAND_LIST_BOARDS);
            send_cmd(sock, req, res);
            printf("\nList Boards:\n%s\n", res);

            printf("Please select Board no (0 to cancel): ");
            scanf("%d", &board_id);
            if (board_id > 0) {
                task_menu(sock, board_id);
            }

        } else if (choice == 2) {
            printf("Please input board name: ");
            scanf(" %[^\n]", text);

            if (isEmptyOrSpace(text)) {
                printf("Error: Board name cannot be empty.\n");
                continue;
            }

            snprintf(req, sizeof(req), "%s%s%s", COMMAND_ADD_BOARD, COMMAND_SEPARATOR, text);
            send_cmd(sock, req, res);
            printf("Response: %s\n", res);

        } else if (choice == 3) {
            printf("Please input board id to update: ");
            scanf("%d", &board_id);
            printf("Please input new board name: ");
            scanf(" %[^\n]", text);

            if (isEmptyOrSpace(text)) {
                printf("Error: Board name cannot be empty.\n");
                continue;
            }

            snprintf(req, sizeof(req), "%s%s%d%s%s", 
                     COMMAND_UPDATE_BOARD, COMMAND_SEPARATOR, board_id, COMMAND_SEPARATOR, text);
            send_cmd(sock, req, res);
            printf("Response: %s\n", res);

        } else if (choice == 4) {
            printf("Please input board id to delete: ");
            scanf("%d", &board_id);

            snprintf(req, sizeof(req), "%s%s%d", COMMAND_DELETE_BOARD, COMMAND_SEPARATOR, board_id);
            send_cmd(sock, req, res);
            printf("Response: %s\n", res);

        } else if (choice == 5) {
            printf("Exiting...\n");
            break;
        }
    }

    close(sock);
    return 0;
}