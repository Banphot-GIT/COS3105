#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/protocol.h"
#include "../include/board.h"
#include "../include/task.h"

void cleanup_test_files(void) {
    remove("data/boards.txt");
    remove("data/tasks.txt");
}

void test_board(void) {
    printf("[TEST] Testing Board CRUD...\n");

    // 1. ทดสอบ Input ว่าง ต้องคืน STATUS_INVALID_ARGUMENTS (200)
    assert(add_board("") == STATUS_INVALID_ARGUMENTS);
    assert(add_board(NULL) == STATUS_INVALID_ARGUMENTS);

    // 2. ทดสอบเพิ่มบอร์ดใหม่
    assert(add_board("Project Alpha") == STATUS_OK);
    assert(add_board("Project Beta") == STATUS_OK);

    // 3. ทดสอบแก้ไขบอร์ด (แก้ ID 1)
    assert(update_board(1, "Project Alpha (Updated)") == STATUS_OK);
    // แก้ไข ID ที่ไม่มีอยู่จริง ต้องคืน STATUS_BOARD_NOT_FOUND (201)
    assert(update_board(999, "None") == STATUS_BOARD_NOT_FOUND);

    // 4. ทดสอบลบบอร์ด
    assert(del_board(2) == STATUS_OK);
    assert(del_board(999) == STATUS_BOARD_NOT_FOUND);

    printf("[PASS] Board CRUD passed.\n");
}

void test_task(void) {
    printf("[TEST] Testing Task CRUD...\n");

    // 1. ทดสอบ Argument ไม่ถูกต้อง (board_id <= 0 หรือชื่อว่าง)
    assert(add_task(0, "Invalid Board ID") == STATUS_INVALID_ARGUMENTS);
    assert(add_task(1, "") == STATUS_INVALID_ARGUMENTS);

    // 2. ทดสอบเพิ่ม Task ให้ Board 1
    assert(add_task(1, "Setup Database") == STATUS_OK);
    assert(add_task(1, "Write API Docs") == STATUS_OK);

    // 3. ทดสอบแก้ไข Task
    assert(update_task(1, 1, "Setup Database (Done)") == STATUS_OK);
    // ระบุ Task ID หรือ Board ID ที่ไม่มีอยู่จริง
    assert(update_task(1, 999, "None") == STATUS_TASK_NOT_FOUND);
    assert(update_task(999, 1, "None") == STATUS_TASK_NOT_FOUND);

    // 4. ทดสอบลบ Task
    assert(del_task(1, 2) == STATUS_OK);
    assert(del_task(1, 999) == STATUS_TASK_NOT_FOUND);

    printf("[PASS] Task CRUD passed.\n");
}

int main(void) {
    cleanup_test_files();

    test_board();
    test_task();

    printf("\n=== All Model Tests Passed Successfully! ===\n");
    return 0;
}