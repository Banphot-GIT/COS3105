#ifndef TASK_H
#define TASK_H

#define task_file "data/tasks.txt"
#define max_task_title 100

int init_task(void);
int get_next_task(void);
int add_task(int board_id, const char *title);
int get_task_board(int board_id);
int update_task(int board_id, int t_task_id, const char *new_title);
int del_task(int board_id, int t_task_id);

#endif