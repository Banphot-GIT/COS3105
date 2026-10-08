#ifndef PROTOCOL_H
#define PROTOCOL_H

// Success & Basic Codes
#define STATUS_OK                 0
#define STATUS_FAIL               1
#define STATUS_UNKNOWN            3

// Client / Request Errors (200+)
#define STATUS_INVALID_ARGUMENTS  200
#define STATUS_BOARD_NOT_FOUND    201
#define STATUS_EMPTY_BOARD        202
#define STATUS_TASK_NOT_FOUND     203
#define STATUS_EMPTY_TASK         204

// Server / System Errors (500+)
#define STATUS_SAVE_BOARD_FAIL    500
#define STATUS_ADD_BOARD_FAIL     501
#define STATUS_SAVE_TASK_FAIL     502
#define STATUS_ADD_TASK_FAIL      503

#define COMMAND_SEPARATOR ","

// Command names
#define COMMAND_ADD_BOARD     "ADD_BOARD"
#define COMMAND_LIST_BOARDS   "LIST_BOARDS"
#define COMMAND_UPDATE_BOARD  "UPDATE_BOARD"
#define COMMAND_DELETE_BOARD  "DELETE_BOARD"

#define COMMAND_ADD_TASK      "ADD_TASK"
#define COMMAND_LIST_TASKS    "LIST_TASKS"
#define COMMAND_UPDATE_TASK   "UPDATE_TASK"
#define COMMAND_DELETE_TASK   "DELETE_TASK"

#define COMMAND_UNKNOWN       "UNKNOWN"
#endif