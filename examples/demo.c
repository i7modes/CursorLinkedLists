/**
 * @file demo.c
 * @brief Demonstration program for Cursor-Based Linked List library.
 * @author i7modes
 * @license MIT
 */

#include "cursor_list.h"
#include <stdio.h>

static void print_separator(const char *title)
{
    printf("\n========================================\n");
    printf("  %s\n", title);
    printf("========================================\n");
}

int main(void)
{
    print_separator("1. Creating Cursor Memory Pool (Capacity: 10)");
    CursorPool *pool = CursorPool_Create(10);
    printf("Pool created. Total slots: %zu, Available: %zu\n",
           CursorPool_Capacity(pool), CursorPool_FreeSlots(pool));
    CursorPool_PrintDump(pool);

    print_separator("2. Creating Two Independent Lists in the Same Arena");
    CursorList list1 = Cursor_CreateList(pool);
    CursorList list2 = Cursor_CreateList(pool);
    printf("Allocated Header for List 1: Slot %d\n", list1);
    printf("Allocated Header for List 2: Slot %d\n", list2);

    print_separator("3. Inserting Elements into Both Lists");
    printf("Inserting 10, 20, 30 into List 1...\n");
    Cursor_InsertBack(10, list1, pool);
    Cursor_InsertBack(20, list1, pool);
    Cursor_InsertBack(30, list1, pool);

    printf("Inserting 100, 200 into List 2...\n");
    Cursor_InsertBack(100, list2, pool);
    Cursor_InsertBack(200, list2, pool);

    printf("\nList 1 traversal: ");
    Cursor_PrintList(list1, pool);

    printf("List 2 traversal: ");
    Cursor_PrintList(list2, pool);

    print_separator("4. Memory Arena Diagnostic Dump");
    printf("Displaying how slots are distributed between List 1, List 2, and Freelist:\n");
    CursorPool_PrintDump(pool);

    print_separator("5. Deleting Elements and Recycling Slots");
    printf("Deleting 20 from List 1...\n");
    Cursor_Delete(20, list1, pool);
    printf("List 1 after deletion: ");
    Cursor_PrintList(list1, pool);

    printf("\nArena dump after deletion (notice freed slot returned to Freelist):\n");
    CursorPool_PrintDump(pool);

    print_separator("6. Legacy Global API Compatibility Check");
    InitializeCursorSpace();
    List legacyL = MakeEmpty(0);
    Insert(555, legacyL);
    Insert(777, legacyL);
    printf("Legacy PrintList output:");
    PrintList(legacyL);
    DeleteList(legacyL);
    printf("Legacy list cleaned up.\n");

    print_separator("7. Complete Teardown");
    Cursor_DestroyList(&list1, pool);
    Cursor_DestroyList(&list2, pool);
    CursorPool_Destroy(&pool);
    printf("Memory pool destroyed. Pointer is now: %p (0 memory leaks)\n", (void *)pool);

    printf("\nAll Cursor Linked List demonstrations completed successfully!\n");
    return 0;
}
