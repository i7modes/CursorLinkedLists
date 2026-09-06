/**
 * @file test_cursor_list.c
 * @brief Automated unit test suite for Cursor-Based Linked List library.
 * @author i7modes
 * @license MIT
 */

#include "cursor_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static int total_tests = 0;
static int passed_tests = 0;

#define TEST_ASSERT(expr, msg) do { \
    total_tests++; \
    if (expr) { \
        passed_tests++; \
        printf("  [PASS] %s\n", msg); \
    } else { \
        printf("  [FAIL] %s (Line %d)\n", msg, __LINE__); \
    } \
} while (0)

static void test_pool_creation_and_exhaustion(void)
{
    printf("\n--- Test: Pool Creation & Allocation Safety ---\n");
    CursorPool *pool = CursorPool_Create(5);
    TEST_ASSERT(pool != NULL, "CursorPool_Create(5) returns non-NULL");
    TEST_ASSERT(CursorPool_Capacity(pool) == 5, "Capacity is 5");
    TEST_ASSERT(CursorPool_FreeSlots(pool) == 4, "Free slots count is 4 (slot 0 reserved)");

    // Allocate all 4 available slots
    CursorPos p1 = Cursor_Alloc(pool);
    CursorPos p2 = Cursor_Alloc(pool);
    CursorPos p3 = Cursor_Alloc(pool);
    CursorPos p4 = Cursor_Alloc(pool);
    TEST_ASSERT(p1 > 0 && p2 > 0 && p3 > 0 && p4 > 0, "Successfully allocated 4 slots");
    TEST_ASSERT(CursorPool_FreeSlots(pool) == 0, "Free slots count is now 0");

    // Allocation when exhausted should safely return 0 without crashing
    CursorPos p5 = Cursor_Alloc(pool);
    TEST_ASSERT(p5 == 0, "Cursor_Alloc returns 0 safely on pool exhaustion");

    // Free a slot and reallocate
    Cursor_Free(p2, pool);
    TEST_ASSERT(CursorPool_FreeSlots(pool) == 1, "Free slots count increments to 1");

    CursorPos p_realloc = Cursor_Alloc(pool);
    TEST_ASSERT(p_realloc == p2, "Freed slot p2 was recycled and reallocated");

    Cursor_Free(p1, pool);
    Cursor_Free(p3, pool);
    Cursor_Free(p4, pool);
    Cursor_Free(p_realloc, pool);
    TEST_ASSERT(CursorPool_FreeSlots(pool) == 4, "All 4 slots returned to freelist");

    CursorPool_Destroy(&pool);
    TEST_ASSERT(pool == NULL, "CursorPool_Destroy nullifies pointer");

    // Invalid capacity
    TEST_ASSERT(CursorPool_Create(0) == NULL, "CursorPool_Create(0) returns NULL safely");
    TEST_ASSERT(CursorPool_Create(1) == NULL, "CursorPool_Create(1) returns NULL safely");
}

static void test_list_crud_operations(void)
{
    printf("\n--- Test: List Insertions, Deletions & Searches ---\n");
    CursorPool *pool = CursorPool_Create(8);
    CursorList list = Cursor_CreateList(pool);
    TEST_ASSERT(list > 0, "Created list with valid header slot");
    TEST_ASSERT(Cursor_IsEmpty(list, pool) == true, "New list is empty");
    TEST_ASSERT(Cursor_Size(list, pool) == 0, "New list size is 0");

    TEST_ASSERT(Cursor_InsertBack(10, list, pool) == true, "InsertBack 10");
    TEST_ASSERT(Cursor_InsertBack(20, list, pool) == true, "InsertBack 20");
    TEST_ASSERT(Cursor_InsertFront(5, list, pool) == true, "InsertFront 5"); // [5, 10, 20]
    TEST_ASSERT(Cursor_Size(list, pool) == 3, "List size is 3");

    int buffer[4];
    size_t copied = Cursor_ToArray(list, pool, buffer, 4);
    TEST_ASSERT(copied == 3, "Copied 3 items to array");
    TEST_ASSERT(buffer[0] == 5 && buffer[1] == 10 && buffer[2] == 20,
                "Array matches order: 5, 10, 20");

    // Find and Insert after
    CursorPos node10 = Cursor_Find(10, list, pool);
    TEST_ASSERT(node10 > 0, "Found node with value 10");
    TEST_ASSERT(Cursor_Insert(15, node10, pool) == true, "Insert 15 after 10"); // [5, 10, 15, 20]
    TEST_ASSERT(Cursor_Size(list, pool) == 4, "Size is 4");

    // Deletion
    TEST_ASSERT(Cursor_Delete(15, list, pool) == true, "Deleted value 15");
    TEST_ASSERT(Cursor_Size(list, pool) == 3, "Size decreases to 3");
    TEST_ASSERT(Cursor_Find(15, list, pool) == 0, "15 is no longer found");

    TEST_ASSERT(Cursor_Delete(999, list, pool) == false, "Delete non-existent value returns false");

    // Clear list
    Cursor_ClearList(list, pool);
    TEST_ASSERT(Cursor_IsEmpty(list, pool) == true, "List is empty after Cursor_ClearList");
    TEST_ASSERT(Cursor_Size(list, pool) == 0, "Size is 0 after ClearList");

    Cursor_DestroyList(&list, pool);
    TEST_ASSERT(list == 0, "Cursor_DestroyList sets list variable to 0");

    CursorPool_Destroy(&pool);
}

static void test_multiple_lists_in_same_pool(void)
{
    printf("\n--- Test: Multiple Independent Lists Sharing Same Arena ---\n");
    CursorPool *pool = CursorPool_Create(10);

    CursorList listA = Cursor_CreateList(pool);
    CursorList listB = Cursor_CreateList(pool);

    Cursor_InsertBack(1, listA, pool);
    Cursor_InsertBack(2, listA, pool);

    Cursor_InsertBack(100, listB, pool);
    Cursor_InsertBack(200, listB, pool);
    Cursor_InsertBack(300, listB, pool);

    TEST_ASSERT(Cursor_Size(listA, pool) == 2, "List A size is 2");
    TEST_ASSERT(Cursor_Size(listB, pool) == 3, "List B size is 3");

    int bufA[2];
    int bufB[3];
    Cursor_ToArray(listA, pool, bufA, 2);
    Cursor_ToArray(listB, pool, bufB, 3);

    TEST_ASSERT(bufA[0] == 1 && bufA[1] == 2, "List A contents: 1, 2");
    TEST_ASSERT(bufB[0] == 100 && bufB[1] == 200 && bufB[2] == 300, "List B contents: 100, 200, 300");

    // Deleting from List A doesn't affect List B
    Cursor_Delete(1, listA, pool);
    TEST_ASSERT(Cursor_Size(listA, pool) == 1, "List A size is 1 after deleting 1");
    TEST_ASSERT(Cursor_Size(listB, pool) == 3, "List B size remains 3");

    Cursor_DestroyList(&listA, pool);
    Cursor_DestroyList(&listB, pool);
    TEST_ASSERT(CursorPool_FreeSlots(pool) == 9, "All 9 arena slots returned after both lists destroyed");

    CursorPool_Destroy(&pool);
}

int main(void)
{
    printf("========================================\n");
    printf("  Cursor Linked List Automated Tests\n");
    printf("========================================\n");

    test_pool_creation_and_exhaustion();
    test_list_crud_operations();
    test_multiple_lists_in_same_pool();

    printf("\n========================================\n");
    printf("  Results: %d/%d assertions passed\n", passed_tests, total_tests);
    printf("========================================\n");

    return (passed_tests == total_tests) ? 0 : 1;
}
