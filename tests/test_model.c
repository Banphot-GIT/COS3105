#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/protocol.h"
#include "../include/board.h"
#include "../include/task.h"

static int total_tests = 0;
static int passed_tests = 0;

static void check_test(const char *case_name, int condition) {
    total_tests++;
    if (condition) {
        printf("  [PASS] %s\n", case_name);
        passed_tests++;
    } else {
        printf("  [FAIL] %s\n", case_name);
    }
}

void cleanup_test_files(void) {
    remove("data/boards.txt");
    remove("data/tasks.txt");
}

void test_board(void) {
    printf("--- Testing Board CRUD ---\n");

    // 1. Validation Test
    check_test("Add board with empty string -> STATUS_INVALID_ARGUMENTS",
               add_board("") == STATUS_INVALID_ARGUMENTS);
    check_test("Add board with NULL pointer -> STATUS_INVALID_ARGUMENTS",
               add_board(NULL) == STATUS_INVALID_ARGUMENTS);

    // 2. Creation Test
    check_test("Add board 'Project Alpha' -> STATUS_OK",
               add_board("Project Alpha") == STATUS_OK);
    check_test("Add board 'Project Beta' -> STATUS_OK",
               add_board("Project Beta") == STATUS_OK);

    // 3. Update Test
    check_test("Update board ID 1 title -> STATUS_OK",
               update_board(1, "Project Alpha (Updated)") == STATUS_OK);
    check_test("Update non-existing board ID 999 -> STATUS_BOARD_NOT_FOUND",
               update_board(999, "None") == STATUS_BOARD_NOT_FOUND);

    // 4. Deletion Test
    check_test("Delete existing board ID 2 -> STATUS_OK",
               del_board(2) == STATUS_OK);
    check_test("Delete non-existing board ID 999 -> STATUS_BOARD_NOT_FOUND",
               del_board(999) == STATUS_BOARD_NOT_FOUND);

    printf("\n");
}

void test_task(void) {
    printf("--- Testing Task CRUD ---\n");

    // 1. Validation Test
    check_test("Add task with invalid board_id (0) -> STATUS_INVALID_ARGUMENTS",
               add_task(0, "Invalid Board ID") == STATUS_INVALID_ARGUMENTS);
    check_test("Add task with empty title -> STATUS_INVALID_ARGUMENTS",
               add_task(1, "") == STATUS_INVALID_ARGUMENTS);

    // 2. Creation Test
    check_test("Add task 'Setup Database' to Board 1 -> STATUS_OK",
               add_task(1, "Setup Database") == STATUS_OK);
    check_test("Add task 'Write API Docs' to Board 1 -> STATUS_OK",
               add_task(1, "Write API Docs") == STATUS_OK);

    // 3. Update Test
    check_test("Update task ID 1 title in Board 1 -> STATUS_OK",
               update_task(1, 1, "Setup Database (Done)") == STATUS_OK);
    check_test("Update non-existing task ID 999 in Board 1 -> STATUS_TASK_NOT_FOUND",
               update_task(1, 999, "None") == STATUS_TASK_NOT_FOUND);
    check_test("Update task in non-existing Board 999 -> STATUS_TASK_NOT_FOUND",
               update_task(999, 1, "None") == STATUS_TASK_NOT_FOUND);

    // 4. Deletion Test
    check_test("Delete existing task ID 2 from Board 1 -> STATUS_OK",
               del_task(1, 2) == STATUS_OK);
    check_test("Delete non-existing task ID 999 from Board 1 -> STATUS_TASK_NOT_FOUND",
               del_task(1, 999) == STATUS_TASK_NOT_FOUND);

    printf("\n");
}

int main(void) {
    cleanup_test_files();

    test_board();
    test_task();

    printf("========================================\n");
    printf("Test Summary: %d/%d passed\n", passed_tests, total_tests);
    printf("========================================\n");

    if (passed_tests == total_tests) {
        printf("RESULT: All tests passed successfully.\n");
        return 0;
    } else {
        printf("RESULT: Some tests failed!\n");
        return 1;
    }
}