#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/task.h"
#include "../include/protocol.h"
#include "board.h"
#include "task.h"
#include "common.h"

int init_task(void){
    FILE *fp = fopen(task_file,"a");
    if(!fp){
        return STATUS_SAVE_TASK_FAIL;
    }
    fclose(fp);
    return STATUS_OK;
}

int get_next_task(void){
    init_task();
    FILE *fp = fopen(task_file, "r");
    if(!fp){
        return 1;
    }
    int bid, tid;
    int maxid = 0;
    char title[max_task_title];

    while (fscanf(fp, "%d,%d,%99[^\n]\n", &bid, &tid, title) == 3){
        if(tid > maxid){
            maxid = tid;
        }
    }
    fclose(fp);
    return maxid +1;
    
}

int add_task(int board_id, const char *title) {

    if (!title || isEmptyOrSpace(title) || board_id <= 0) {
        return STATUS_INVALID_ARGUMENTS;
    }

    if (strchr(title, ',') != NULL) {
        return STATUS_INVALID_ARGUMENTS;
    }

    FILE *f_board = fopen(board_file, "r");
    if (!f_board) {
        return STATUS_BOARD_NOT_FOUND;
    }
    char b_line[256];
    int board_exists = 0;
    while (fgets(b_line, sizeof(b_line), f_board)) {
        char *b_id_str = strtok(b_line, ",\r\n");
        if (b_id_str && atoi(b_id_str) == board_id) {
            board_exists = 1;
            break;
        }
    }
    fclose(f_board);

    if (!board_exists) {
        return STATUS_BOARD_NOT_FOUND;
    }

    init_task();
    int newid = get_next_task();

    FILE *fp = fopen(task_file, "a");
    if (!fp) {
        return STATUS_ADD_TASK_FAIL;
    }
    fprintf(fp, "%d,%d,%s\n", board_id, newid, title);
    fclose(fp);

    return STATUS_OK;
}

int get_task_board(int board_id){
    if(board_id <=0){
        return STATUS_INVALID_ARGUMENTS;
    }

    init_task();
    FILE *fp = fopen(task_file,"r");
    if(!fp){
        return STATUS_FAIL;
    }

    int bid, tid;
    char title[max_task_title];
    int count = 0;

    printf("\n-- List Task in Board no. %d --\n", board_id);
    while (fscanf(fp, "%d,%d,%99[^\n]\n", &bid, &tid, title) ==3)
    {
        if(bid == board_id){
            printf("%d. %s\n", tid, title);
            count++;
        }
    }
    fclose(fp);

    if(count == 0){
        printf("No tasks found in this board.\n");
        return STATUS_EMPTY_TASK;
    }

    return STATUS_OK;
}

int update_task(int board_id, int t_task_id, const char *new_title) {
    if (!new_title || isEmptyOrSpace(new_title) || board_id <= 0 || t_task_id <= 0) {
        return STATUS_INVALID_ARGUMENTS;
    }
    if (strchr(new_title, ',') != NULL) {
        return STATUS_INVALID_ARGUMENTS;
    }

    FILE *f_board = fopen(board_file, "r");
    if (!f_board) {
        return STATUS_BOARD_NOT_FOUND;
    }
    char b_line[256];
    int board_exists = 0;
    while (fgets(b_line, sizeof(b_line), f_board)) {
        char *b_id_str = strtok(b_line, ",\r\n");
        if (b_id_str && atoi(b_id_str) == board_id) {
            board_exists = 1;
            break;
        }
    }
    fclose(f_board);

    if (!board_exists) {
        return STATUS_BOARD_NOT_FOUND;
    }

    init_task();
    FILE *fp = fopen(task_file, "r");
    if (!fp) {
        return STATUS_FAIL;
    }

    FILE *temp = fopen("data/temp_task.txt", "w");
    if (!temp) {
        fclose(fp);
        return STATUS_SAVE_TASK_FAIL;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        replace_char(line, '\r', '\0');
        replace_char(line, '\n', '\0');
        if (isEmptyOrSpace(line)) continue;

        char line_copy[256];
        strncpy(line_copy, line, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        char *saveptr;
        char *bid_str = strtok_r(line_copy, ",", &saveptr);
        char *tid_str = strtok_r(NULL, ",", &saveptr);
        char *title   = strtok_r(NULL, ",", &saveptr);

        if (bid_str && tid_str && title) {
            int bid = atoi(bid_str);
            int tid = atoi(tid_str);

            if (bid == board_id && tid == t_task_id) {
                fprintf(temp, "%d,%d,%s\n", bid, tid, new_title);
                found = 1;
            } else {
                fprintf(temp, "%d,%d,%s\n", bid, tid, title);
            }
        }
    }

    fclose(fp);
    fclose(temp);

    remove(task_file);
    rename("data/temp_task.txt", task_file);

    if (!found) {
        return STATUS_TASK_NOT_FOUND;
    }

    return STATUS_OK;
}

int del_task(int board_id, int t_task_id) {
    if (board_id <= 0 || t_task_id <= 0) {
        return STATUS_INVALID_ARGUMENTS;
    }

    FILE *fp = fopen(task_file, "r");
    if (!fp) {
        return STATUS_TASK_NOT_FOUND;
    }

    FILE *temp = fopen("data/temp_task.txt", "w");
    if (!temp) {
        fclose(fp);
        return STATUS_FAIL;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        replace_char(line, '\r', '\0');
        replace_char(line, '\n', '\0');
        if (isEmptyOrSpace(line)) continue;

        char line_copy[256];
        strncpy(line_copy, line, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        char *saveptr;
        char *bid_str = strtok_r(line_copy, ",", &saveptr);
        char *tid_str = strtok_r(NULL, ",", &saveptr);
        char *title   = strtok_r(NULL, ",", &saveptr);

        if (bid_str && tid_str && title) {
            int bid = atoi(bid_str);
            int tid = atoi(tid_str);

            if (bid == board_id && tid == t_task_id) {
                found = 1;
            } else {
                fprintf(temp, "%d,%d,%s\n", bid, tid, title);
            }
        }
    }

    fclose(fp);
    fclose(temp);
    remove(task_file);
    rename("data/temp_task.txt", task_file);

    if (!found) {
        return STATUS_TASK_NOT_FOUND;
    }

    return STATUS_OK;
}