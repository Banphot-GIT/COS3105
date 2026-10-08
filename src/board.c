#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/board.h"
#include "../include/protocol.h"

int init_board(void){
    FILE *fp = fopen(board_file, "a");
    if (!fp) {
        return STATUS_SAVE_BOARD_FAIL;
    }
    fclose(fp);
    return STATUS_OK;
}

int get_next_board(void){
    FILE *fp = fopen(board_file, "r");
    if (!fp) {
        return 1;
    }
    int id, max_id = 0;
    char name[max_board_name];

    while (fscanf(fp, "%d,%63[^\n]\n", &id, name) == 2){
        if(id > max_id){
            max_id = id;
        }
    }
    fclose(fp);
    return max_id +1;
}

int add_board(const char *name){
    if(!name || strlen(name) == 0){
        return STATUS_INVALID_ARGUMENTS;
    }
    init_board();
    int new_id = get_next_board();

    FILE *fp = fopen(board_file, "a");
    if (!fp) {
        return STATUS_ADD_BOARD_FAIL;
    }
    fprintf(fp, "%d,%s\n", new_id, name);
    fclose(fp);

    return STATUS_OK;
}

int get_all_board(void){
    init_board();
    FILE *fp = fopen(board_file, "r");
    if(fp == NULL){
        return STATUS_FAIL;
    }

    int id;
    char name[max_board_name];
    int count = 0;

    printf("\n-- List Boards --\n");
    while (fscanf(fp, "%d, %99[^\n]\n", &id, name) == 2){
        printf("%d. %s\n", id, name);
        count++;
    }
    fclose(fp);

    if(count == 0){
        printf("No board found\n");
        return STATUS_EMPTY_BOARD;
    }
    return STATUS_OK;
}

int update_board(int target_id, const char *new_name){
    if (new_name == NULL || strlen(new_name) == 0){
        return STATUS_INVALID_ARGUMENTS;
    }

    init_board();
    FILE *fp = fopen(board_file, "r");
    if(fp == NULL){
        return STATUS_FAIL;
    }

    FILE *temp_fp = fopen("data/temp.txt", "w");
    if(temp_fp == NULL){
        fclose(fp);
        return STATUS_SAVE_BOARD_FAIL;
    }

    int id;
    char name[max_board_name];
    int found = 0;
    while (fscanf(fp, "%d, %99[^\n]\n", &id, name) ==2)
    {
        if(id == target_id){
            fprintf(temp_fp, "%d,%s\n", id, new_name);
            found = 1;
        } else{
            fprintf(temp_fp, "%d,%s\n", id, name);
        }
    }
    fclose(fp);
    fclose(temp_fp);

    remove(board_file);
    rename("data/temp.txt", board_file);

    if(!found){
        return STATUS_BOARD_NOT_FOUND;
    }
    return STATUS_OK;
}

int del_board(int target_id){
    init_board();
    FILE *fp = fopen(board_file,"r");
    if(fp == NULL){
        return STATUS_FAIL;
    }

    FILE *temp_fp = fopen("data/temp.txt", "w");
    if(temp_fp == NULL){
        fclose(fp);
        return STATUS_SAVE_BOARD_FAIL;
    }

    int id;
    char name[max_board_name];
    int found = 0;
    while (fscanf(fp, "%d,%99[^\n]\n", &id, name) == 2)
    {
        if(id== target_id){
            found = 1;
        }else{
            fprintf(temp_fp, "%d,%s\n", id, name);
        }
    }
    fclose(fp);
    fclose(temp_fp);
    
    remove(board_file);
    rename("data/temp.txt", board_file);

    if(!found){
        return STATUS_BOARD_NOT_FOUND;
    }
    return STATUS_OK;
}