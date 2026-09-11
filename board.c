#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

//get next board id
static int next_boardid(){
    FILE *fp = fopen("board.txt","r");
    if (!fp) return 0;
    int id = 0;
    int maxid = 0;
    char item[64];
    while (fscanf(fp, "%d,%63[^\n]\n", &id, item)==2)
    {
        if(id>maxid){
            maxid = id;
        }
    }
    fclose(fp);
    return maxid+1;
}

//add new board
int add_board(const char *newboard){
    if(!newboard || strlen(newboard)==0){
        return 0;
    }
    int newid = next_boardid();
    FILE *fp = fopen("board.txt","a");
    if (!fp) {
        return 1;
    }
    fprintf(fp,"%d,%s\n", newid, newboard);
    fclose(fp);
    return newid;
}

//read board
int read_board(board *board, int maxcount){
    FILE *fp = fopen("board.txt","r");
    if(!fp){
        return 0;
    }
    int count = 0;
    while (count < maxcount && 
        fscanf(fp,"%d,%63[^\n]\n", &board[count].id, board[count].bname)==2)
    {
       count++;
    }
    fclose(fp);
    return count;
}

//update board
int update_board(int target, const char *update){
    FILE *fp = fopen("board.txt", "r");
    if(!fp){
        return 0;
    }
    
}

//test