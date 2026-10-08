#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/task.h"
#include "../include/protocol.h"

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

int add_task(int board_id, const char *title){
    if(!title || strlen(title) == 0 || board_id <=0){
        return STATUS_INVALID_ARGUMENTS;
    }
    init_task();
    int newid = get_next_task();

    FILE *fp = fopen(task_file, "a");
    if(!fp){
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

int update_task(int board_id, int t_task_id, const char *new_title){
    if(!new_title || strlen(new_title) ==0 || board_id <=0 || t_task_id <=0){
        return STATUS_INVALID_ARGUMENTS;
    }

    init_task();
    FILE *fp = fopen(task_file,"r");
    if(!fp){
        return STATUS_FAIL;
    }

    FILE *temp = fopen("data/temp_task.txt", "w");
    if(!temp){
        fclose(fp);
        return STATUS_SAVE_TASK_FAIL;
    }

    int bid, tid;
    char title[max_task_title];
    int found = 0;

    while (fscanf(fp, "%d,%d,%99[^\n]\n", &bid, &tid, title) == 3)
    {
        if(bid == board_id && tid == t_task_id){
            fprintf(temp, "%d,%d,%s\n", bid, tid, new_title);
            found = 1;
        }else{
            fprintf(temp, "%d,%d,%s\n", bid, tid, title);
        }
    }
    fclose(fp);
    fclose(temp);

    remove(task_file);
    rename("data/temp_task.txt", task_file);

    if(!found){
        return STATUS_TASK_NOT_FOUND;
    }
        return STATUS_OK;
    
}

int del_task(int board_id, int t_task_id){
    if(board_id <=0 || t_task_id <=0){
        return STATUS_INVALID_ARGUMENTS;
    }
    init_task();
    FILE *fp = fopen(task_file, "r");
    if(!fp){
        return STATUS_FAIL;
    }

    FILE *temp = fopen("data/temp_task.txt", "w");
    if(!temp){
        fclose(fp);
        return STATUS_SAVE_TASK_FAIL;
    }
    int bid, tid;
    char title[max_task_title];
    int found = 0;

    while (fscanf(fp, "%d,%d,%99[^\n]\n", &bid, &tid, title) == 3)
    {
        if(bid==board_id && tid == t_task_id){
            found = 1;
        }else{
            fprintf(temp, "%d,%d,%s\n", bid, tid, title);
        }
    }
    fclose(fp);
    fclose(temp);

    remove(task_file);
    rename("data/temp_task.txt", task_file);

    if(!found){
        return STATUS_TASK_NOT_FOUND;
    }
    return STATUS_OK;
    
}