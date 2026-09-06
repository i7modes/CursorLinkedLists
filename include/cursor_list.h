/**
 * @file cursor_list.h
 * @brief Cursor-Based Linked List (Array Memory Pool with Freelist) in C.
 * @author i7modes
 * @license MIT
 */

#ifndef CURSOR_LIST_H
#define CURSOR_LIST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int CursorPos;
typedef int CursorList;

#define CURSOR_NULL 0
#define DEFAULT_SPACE_SIZE 11

/**
 * @brief Node stored in the cursor array.
 */
typedef struct CursorNode {
    int element;            /**< Data payload */
    int next;               /**< Index of the next node in the pool (0 = end/NULL) */
} CursorNode;

/**
 * @brief Memory pool (arena) encapsulating array storage and freelist management.
 */
typedef struct CursorPool {
    CursorNode *nodes;      /**< Node array buffer */
    size_t capacity;        /**< Total number of slots in the pool */
    size_t free_count;      /**< Number of available slots currently in the freelist */
} CursorPool;

/* -------------------------------------------------------------------------- */
/*                          Memory Pool Management                            */
/* -------------------------------------------------------------------------- */

/**
 * @brief Creates a new memory pool arena. Slot 0 is reserved as the freelist head.
 * @param capacity Number of slots to allocate (must be >= 2).
 * @return Pointer to new CursorPool, or NULL on allocation failure.
 */
CursorPool* CursorPool_Create(size_t capacity);

/**
 * @brief Deallocates the memory pool and its node buffer, setting *pool_ptr to NULL.
 * @param pool_ptr Pointer to the CursorPool pointer.
 */
void CursorPool_Destroy(CursorPool **pool_ptr);

/**
 * @brief Returns the number of currently available free slots in the pool.
 */
size_t CursorPool_FreeSlots(const CursorPool *pool);

/**
 * @brief Returns the total capacity of the memory pool.
 */
size_t CursorPool_Capacity(const CursorPool *pool);

/**
 * @brief Prints a full diagnostic dump of all memory slots (Index, Element, Next, Status).
 * @param pool Pointer to the memory pool.
 */
void CursorPool_PrintDump(const CursorPool *pool);

/* -------------------------------------------------------------------------- */
/*                       Freelist Allocation & Freeing                        */
/* -------------------------------------------------------------------------- */

/**
 * @brief Allocates an available slot from the pool's freelist (simulates malloc).
 * @param pool Pointer to the memory pool.
 * @return Slot index > 0 on success, or 0 (CURSOR_NULL) if the pool is exhausted.
 */
CursorPos Cursor_Alloc(CursorPool *pool);

/**
 * @brief Returns an in-use slot back to the freelist (simulates free).
 * @param pos Slot index to release.
 * @param pool Pointer to the memory pool.
 */
void Cursor_Free(CursorPos pos, CursorPool *pool);

/* -------------------------------------------------------------------------- */
/*                           List Operations                                  */
/* -------------------------------------------------------------------------- */

/**
 * @brief Allocates and initializes a new list with a header node in the pool.
 * @param pool Pointer to the memory pool.
 * @return Header slot index > 0, or 0 if pool is full.
 */
CursorList Cursor_CreateList(CursorPool *pool);

/**
 * @brief Clears all payload nodes from a list, recycling them back to the freelist.
 * Retains the header node.
 * @param list Header index of the list.
 * @param pool Pointer to the memory pool.
 */
void Cursor_ClearList(CursorList list, CursorPool *pool);

/**
 * @brief Completely destroys a list, freeing all payload nodes and the header node.
 * Sets *list_ptr to 0 (CURSOR_NULL).
 */
void Cursor_DestroyList(CursorList *list_ptr, CursorPool *pool);

/**
 * @brief Checks if a list contains any elements (excluding the sentinel header).
 */
bool Cursor_IsEmpty(CursorList list, const CursorPool *pool);

/**
 * @brief Checks if a position is the last node in a list.
 */
bool Cursor_IsLast(CursorPos pos, const CursorPool *pool);

/**
 * @brief Returns the number of elements in a list.
 */
size_t Cursor_Size(CursorList list, const CursorPool *pool);

/**
 * @brief Searches for the first node containing the specified value in the list.
 * @return Slot index of matching node, or 0 if not found.
 */
CursorPos Cursor_Find(int value, CursorList list, const CursorPool *pool);

/**
 * @brief Searches for the node preceding the first node with the specified value.
 * @return Slot index of predecessor node, or last node if not found.
 */
CursorPos Cursor_FindPrevious(int value, CursorList list, const CursorPool *pool);

/**
 * @brief Inserts a new value immediately following a specified position slot.
 * @param value The value to insert.
 * @param pos The slot index after which to insert.
 * @param pool Pointer to the memory pool.
 * @return true on success, false if pool is full or pos is invalid.
 */
bool Cursor_Insert(int value, CursorPos pos, CursorPool *pool);

/**
 * @brief Inserts a new value at the front of the list (immediately after header).
 */
bool Cursor_InsertFront(int value, CursorList list, CursorPool *pool);

/**
 * @brief Inserts a new value at the end of the list.
 */
bool Cursor_InsertBack(int value, CursorList list, CursorPool *pool);

/**
 * @brief Deletes the first node containing the specified value from the list,
 * recycling the freed slot back to the freelist.
 * @return true if found and removed, false otherwise.
 */
bool Cursor_Delete(int value, CursorList list, CursorPool *pool);

/**
 * @brief Copies list elements in sequence into an output buffer.
 * @return Number of elements copied.
 */
size_t Cursor_ToArray(CursorList list, const CursorPool *pool, int *buffer, size_t max_len);

/**
 * @brief Prints the linked list elements in order.
 */
void Cursor_PrintList(CursorList list, const CursorPool *pool);

/* -------------------------------------------------------------------------- */
/*                       Legacy Global API Compatibility                      */
/* -------------------------------------------------------------------------- */

#define SPACE_SIZE DEFAULT_SPACE_SIZE

typedef int List;
typedef int Position;

struct node {
    int Element;
    int Next;
};

extern struct node CursorSpace[SPACE_SIZE];

void InitializeCursorSpace(void);
List Legacy_CursorList(void);
Position Legacy_CursorAlloc(void);
void Legacy_CursorFree(Position P);
List MakeEmpty(List L);
int IsEmpty(List L);
int IsLast(Position P, List L);
void Insert(int X, List L);
Position Find(int X, List L);
Position FindPrevious(int X, List L);
void Delete(int X, List L);
void DeleteList(List L);
void PrintList(List L);

#ifdef __cplusplus
}
#endif

#endif /* CURSOR_LIST_H */
