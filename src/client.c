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

void send_cmd(int sock, const char *msg, char *res) {
    char send_buf[BUF_SIZE];
    snprintf(send_buf, sizeof(send_buf), "%s\n", msg);
    send(sock, send_buf, strlen(send_buf), 0);

    memset(res, 0, BUF_SIZE);
    recv(sock, res, BUF_SIZE - 1, 0);
    replace_char(res, '\r', '\0');
    replace_char(res, '\n', '\0');
}

int display_boards(char *res) {
    char copy[BUF_SIZE];
    strncpy(copy, res, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *saveptr;
    char *cmd = strtok_r(copy, COMMAND_SEPARATOR, &saveptr);
    char *status_str = strtok_r(NULL, COMMAND_SEPARATOR, &saveptr);
    char *data = strtok_r(NULL, COMMAND_SEPARATOR, &saveptr);

    (void)cmd;
    if (!status_str) {
        printf("Error: Invalid server response.\n");
        return 0;
    }

    int status = atoi(status_str);
    if (status == STATUS_EMPTY_BOARD || !data || isEmptyOrSpace(data)) {
        printf("(No boards available)\n");
        return 0;
    }

    if (status != STATUS_OK) {
        printf("Failed to list boards (Code: %d)\n", status);
        return 0;
    }

    char *item_saveptr;
    char *item = strtok_r(data, ";", &item_saveptr);
    while (item != NULL) {
        char *colon = strchr(item, ':');
        if (colon) {
            *colon = '\0';
            printf("%s: %s\n", item, colon + 1);
        } else {
            printf("%s\n", item);
        }
        item = strtok_r(NULL, ";", &item_saveptr);
    }
    return 1;
}

int display_tasks(char *res) {
    char copy[BUF_SIZE];
    strncpy(copy, res, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *saveptr;
    char *cmd = strtok_r(copy, COMMAND_SEPARATOR, &saveptr);
    char *status_str = strtok_r(NULL, COMMAND_SEPARATOR, &saveptr);
    char *data = strtok_r(NULL, COMMAND_SEPARATOR, &saveptr);

    (void)cmd;
    if (!status_str) {
        printf("Error: Invalid server response.\n");
        return -1;
    }

    int status = atoi(status_str);

    if (status == STATUS_BOARD_NOT_FOUND) {
        printf("Error: Board not found!\n");
        return 0;
    }

    if (status == STATUS_EMPTY_TASK || !data || isEmptyOrSpace(data)) {
        printf("(No tasks in this board)\n");
        return 1;
    }

    if (status != STATUS_OK) {
        printf("Failed to list tasks (Code: %d)\n", status);
        return -1;
    }

    char *item_saveptr;
    char *item = strtok_r(data, ";", &item_saveptr);
    while (item != NULL) {
        char *colon = strchr(item, ':');
        if (colon) {
            *colon = '\0';
            printf("  Task %s: %s\n", item, colon + 1);
        } else {
            printf("  %s\n", item);
        }
        item = strtok_r(NULL, ";", &item_saveptr);
    }
    return 1;
}

void print_action_result(const char *res, const char *action_name) {
    char copy[BUF_SIZE];
    strncpy(copy, res, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *saveptr;
    strtok_r(copy, COMMAND_SEPARATOR, &saveptr); // ข้ามชื่อ Command
    char *status_str = strtok_r(NULL, COMMAND_SEPARATOR, &saveptr);

    if (!status_str) {
        printf("Response: %s\n", res);
        return;
    }

    int status = atoi(status_str);
    if (status == STATUS_OK) {
        printf("Success: %s completed successfully.\n", action_name);
    } else if (status == STATUS_BOARD_NOT_FOUND) {
        printf("Error: Board not found!\n");
    } else if (status == STATUS_TASK_NOT_FOUND) {
        printf("Error: Task not found!\n");
    } else if (status == STATUS_INVALID_ARGUMENTS) {
        printf("Error: Invalid arguments provided.\n");
    } else {
        printf("Error: %s failed (Code: %d)\n", action_name, status);
    }
}

// Task Menu
void task_menu(int sock, int board_id, const char *board_name) {
    int choice;
    char input_buf[128];
    char req[BUF_SIZE], res[BUF_SIZE];
    char text[256];
    int task_id;

    snprintf(req, sizeof(req), "%s%s%d", COMMAND_LIST_TASKS, COMMAND_SEPARATOR, board_id);
    send_cmd(sock, req, res);

    printf("\n========================================\n");
    printf("List Task in Board no. %d [%s]\n", board_id, board_name);
    if (display_tasks(res) <= 0) {
        return;
    }

    while (1) {
        printf("\nWhat do you want to do?\n");
        printf("1. Create Task\n");
        printf("2. Edit Task\n");
        printf("3. Delete Task\n");
        printf("4. Back to Board Menu\n");
        printf("You select option: ");

        if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
            continue;
        }
        replace_char(input_buf, '\r', '\0');
        replace_char(input_buf, '\n', '\0');

        if (isEmptyOrSpace(input_buf)) {
            printf("Error: Option cannot be empty! Please enter a number (1-4).\n");
            continue;
        }

        if (!isNumeric(input_buf)) {
            printf("Error: Invalid input! Please enter a number (1-4).\n");
            continue;
        }

        choice = atoi(input_buf);

        if (choice == 1) {
            printf("Create Task in Board no. %d [%s]\n", board_id, board_name);
            printf("Please input task title: ");
            if (fgets(text, sizeof(text), stdin) == NULL) continue;
            replace_char(text, '\r', '\0');
            replace_char(text, '\n', '\0');

            if (isEmptyOrSpace(text)) {
                printf("Error: Task title cannot be empty.\n");
                continue;
            }

            snprintf(req, sizeof(req), "%s%s%d%s%s", 
                     COMMAND_ADD_TASK, COMMAND_SEPARATOR, board_id, COMMAND_SEPARATOR, text);
            send_cmd(sock, req, res);
            print_action_result(res, "Create Task");

        } else if (choice == 2) {
            printf("Please input task id to edit: ");
            if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) continue;
            replace_char(input_buf, '\r', '\0');
            replace_char(input_buf, '\n', '\0');

            if (isEmptyOrSpace(input_buf) || !isNumeric(input_buf)) {
                printf("Error: Invalid Task ID!\n");
                continue;
            }
            task_id = atoi(input_buf);

            printf("Please input new task title: ");
            if (fgets(text, sizeof(text), stdin) == NULL) continue;
            replace_char(text, '\r', '\0');
            replace_char(text, '\n', '\0');

            if (isEmptyOrSpace(text)) {
                printf("Error: Task title cannot be empty.\n");
                continue;
            }

            snprintf(req, sizeof(req), "%s%s%d%s%d%s%s", 
                     COMMAND_UPDATE_TASK, COMMAND_SEPARATOR, board_id, COMMAND_SEPARATOR, task_id, COMMAND_SEPARATOR, text);
            send_cmd(sock, req, res);
            print_action_result(res, "Update Task");

        } else if (choice == 3) {
            printf("Please input task id to delete: ");
            if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) continue;
            replace_char(input_buf, '\r', '\0');
            replace_char(input_buf, '\n', '\0');

            if (isEmptyOrSpace(input_buf) || !isNumeric(input_buf)) {
                printf("Error: Invalid Task ID!\n");
                continue;
            }
            task_id = atoi(input_buf);

            snprintf(req, sizeof(req), "%s%s%d%s%d", 
                     COMMAND_DELETE_TASK, COMMAND_SEPARATOR, board_id, COMMAND_SEPARATOR, task_id);
            send_cmd(sock, req, res);
            print_action_result(res, "Delete Task");

        } else if (choice == 4) {
            break;

        } else {
            printf("Error: Option must be between 1 and 4.\n");
            continue;
        }
        printf("\n========================================\n");
        printf("List Task in Board no. %d [%s]\n", board_id, board_name);
        snprintf(req, sizeof(req), "%s%s%d", COMMAND_LIST_TASKS, COMMAND_SEPARATOR, board_id);
        send_cmd(sock, req, res);
        display_tasks(res);
    }
}

void get_board_name_by_id(const char *res, int target_id, char *out_name, size_t max_len) {
    strncpy(out_name, "", max_len);

    char copy[BUF_SIZE];
    strncpy(copy, res, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';

    char *saveptr;
    char *cmd = strtok_r(copy, COMMAND_SEPARATOR, &saveptr);
    char *status_str = strtok_r(NULL, COMMAND_SEPARATOR, &saveptr);
    char *data = strtok_r(NULL, COMMAND_SEPARATOR, &saveptr);
    (void)cmd;

    if (!status_str || atoi(status_str) != STATUS_OK || !data) return;

    char *item_saveptr;
    char *item = strtok_r(data, ";", &item_saveptr);
    while (item != NULL) {
        char *colon = strchr(item, ':');
        if (colon) {
            *colon = '\0';
            if (atoi(item) == target_id) {
                strncpy(out_name, colon + 1, max_len - 1);
                out_name[max_len - 1] = '\0';
                return;
            }
        }
        item = strtok_r(NULL, ";", &item_saveptr);
    }
}

// Board Menu
int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    int choice;
    char input_buf[128];
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

        if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
            continue;
        }
        replace_char(input_buf, '\r', '\0');
        replace_char(input_buf, '\n', '\0');

        if (isEmptyOrSpace(input_buf)) {
            printf("Error: Option cannot be empty! Please enter a number (1-5).\n");
            continue;
        }

        if (!isNumeric(input_buf)) {
            printf("Error: Invalid input! Please enter a number (1-5).\n");
            continue;
        }

        choice = atoi(input_buf);

        if (choice == 1) {
            snprintf(req, sizeof(req), "%s", COMMAND_LIST_BOARDS);
            send_cmd(sock, req, res);

            printf("\nList Boards:\n");
            int has_boards = display_boards(res);

            if (has_boards) {
                printf("\nPlease select Board no (0 to cancel): ");
                if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) continue;
                replace_char(input_buf, '\r', '\0');
                replace_char(input_buf, '\n', '\0');

                if (isEmptyOrSpace(input_buf) || !isNumeric(input_buf)) {
                    printf("Error: Invalid Board ID!\n");
                    continue;
                }

                board_id = atoi(input_buf);
                if (board_id > 0) {
                    char current_board_name[100] = "Unknown";
                    get_board_name_by_id(res, board_id, current_board_name, sizeof(current_board_name));
                    task_menu(sock, board_id, current_board_name);
                }
            }

        } else if (choice == 2) {
            printf("Please input board name: ");
            if (fgets(text, sizeof(text), stdin) == NULL) continue;
            replace_char(text, '\r', '\0');
            replace_char(text, '\n', '\0');

            if (isEmptyOrSpace(text)) {
                printf("Error: Board name cannot be empty.\n");
                continue;
            }

            snprintf(req, sizeof(req), "%s%s%s", COMMAND_ADD_BOARD, COMMAND_SEPARATOR, text);
            send_cmd(sock, req, res);
            print_action_result(res, "Create Board");

        } else if (choice == 3) {
            printf("Please input board id to update: ");
            if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) continue;
            replace_char(input_buf, '\r', '\0');
            replace_char(input_buf, '\n', '\0');

            if (isEmptyOrSpace(input_buf) || !isNumeric(input_buf)) {
                printf("Error: Invalid Board ID!\n");
                continue;
            }
            board_id = atoi(input_buf);

            printf("Please input new board name: ");
            if (fgets(text, sizeof(text), stdin) == NULL) continue;
            replace_char(text, '\r', '\0');
            replace_char(text, '\n', '\0');

            if (isEmptyOrSpace(text)) {
                printf("Error: Board name cannot be empty.\n");
                continue;
            }

            snprintf(req, sizeof(req), "%s%s%d%s%s", 
                     COMMAND_UPDATE_BOARD, COMMAND_SEPARATOR, board_id, COMMAND_SEPARATOR, text);
            send_cmd(sock, req, res);
            print_action_result(res, "Update Board");

        } else if (choice == 4) {
            printf("Please input board id to delete: ");
            if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) continue;
            replace_char(input_buf, '\r', '\0');
            replace_char(input_buf, '\n', '\0');

            if (isEmptyOrSpace(input_buf) || !isNumeric(input_buf)) {
                printf("Error: Invalid Board ID!\n");
                continue;
            }
            board_id = atoi(input_buf);

            snprintf(req, sizeof(req), "%s%s%d", COMMAND_DELETE_BOARD, COMMAND_SEPARATOR, board_id);
            send_cmd(sock, req, res);
            print_action_result(res, "Delete Board");

        } else if (choice == 5) {
            printf("Exiting...\n");
            break;

        } else {
            printf("Error: Option must be between 1 and 5.\n");
        }
    }

    close(sock);
    return 0;
}