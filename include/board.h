#ifndef BOARD_H
#define BOARD_H

#define max_board_name 100
#define max_board 100
#define board_file "data/boards.txt"

typedef struct {
    int id;
    char name[max_board_name];
} Board;

int init_board(void);
int get_next_board(void);
int add_board(const char *name);
int get_all_board(void);
int update_board(int id, const char *new_name);
int del_board(int id);

#endif