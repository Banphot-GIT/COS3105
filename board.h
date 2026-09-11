#ifndef board.h
typedef struct 
{
    int id;
    char bname[64];
} board;

int add_board(const char *item);
int read_board(board *board, int maxcount);
int update_board(int target, const char *newitem);
int delete_board(int target);

#endif