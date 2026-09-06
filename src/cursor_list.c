/**
 * @file cursor_list.c
 * @brief Implementation of Cursor-Based Linked List with Memory Arena Freelist.
 * @author i7modes
 * @license MIT
 */

#include "cursor_list.h"
#include <stdio.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/*                          Memory Pool Management                            */
/* -------------------------------------------------------------------------- */

CursorPool* CursorPool_Create(size_t capacity)
{
    if (capacity < 2)
    {
        return NULL;
    }

    CursorPool *pool = (CursorPool *)malloc(sizeof(CursorPool));
    if (pool == NULL)
    {
        fprintf(stderr, "Error: Out of memory in CursorPool_Create (pool handle).\n");
        return NULL;
    }

    pool->nodes = (CursorNode *)malloc(sizeof(CursorNode) * capacity);
    if (pool->nodes == NULL)
    {
        fprintf(stderr, "Error: Out of memory in CursorPool_Create (nodes buffer).\n");
        free(pool);
        return NULL;
    }

    /* Initialize the freelist chaining through slot 0 */
    for (size_t i = 0; i < capacity - 1; i++)
    {
        pool->nodes[i].element = 0;
        pool->nodes[i].next = (int)(i + 1);
    }
    pool->nodes[capacity - 1].element = 0;
    pool->nodes[capacity - 1].next = 0;

    pool->capacity = capacity;
    pool->free_count = capacity - 1;

    return pool;
}

void CursorPool_Destroy(CursorPool **pool_ptr)
{
    if (pool_ptr == NULL || *pool_ptr == NULL)
    {
        return;
    }

    CursorPool *pool = *pool_ptr;
    if (pool->nodes != NULL)
    {
        free(pool->nodes);
    }
    free(pool);
    *pool_ptr = NULL;
}

size_t CursorPool_FreeSlots(const CursorPool *pool)
{
    return (pool != NULL) ? pool->free_count : 0;
}

size_t CursorPool_Capacity(const CursorPool *pool)
{
    return (pool != NULL) ? pool->capacity : 0;
}

void CursorPool_PrintDump(const CursorPool *pool)
{
    if (pool == NULL)
    {
        printf("(null pool)\n");
        return;
    }

    printf("\n--- Cursor Memory Arena Dump (Capacity: %zu, Free: %zu) ---\n",
           pool->capacity, pool->free_count);
    printf(" Slot | Element | Next | Status\n");
    printf("------+---------+------+-------------------\n");

    for (size_t i = 0; i < pool->capacity; i++)
    {
        const char *desc = "In-Use";
        if (i == 0)
        {
            desc = "Freelist Head";
        }
        else
        {
            /* Check if slot is in freelist */
            int curr = pool->nodes[0].next;
            while (curr != 0)
            {
                if ((size_t)curr == i)
                {
                    desc = "Free";
                    break;
                }
                curr = pool->nodes[curr].next;
            }
        }

        printf("  %3zu |   %5d |  %3d | %s\n",
               i, pool->nodes[i].element, pool->nodes[i].next, desc);
    }
    printf("---------------------------------------------------\n");
}

/* -------------------------------------------------------------------------- */
/*                       Freelist Allocation & Freeing                        */
/* -------------------------------------------------------------------------- */

CursorPos Cursor_Alloc(CursorPool *pool)
{
    if (pool == NULL)
    {
        return 0;
    }

    CursorPos p = pool->nodes[0].next;
    if (p == 0)
    {
        /* Pool exhausted - return 0 safely instead of exiting */
        return 0;
    }

    pool->nodes[0].next = pool->nodes[p].next;
    pool->nodes[p].next = 0;
    pool->nodes[p].element = 0;
    pool->free_count--;

    return p;
}

void Cursor_Free(CursorPos pos, CursorPool *pool)
{
    if (pool == NULL || pos <= 0 || (size_t)pos >= pool->capacity)
    {
        return;
    }

    pool->nodes[pos].element = 0;
    pool->nodes[pos].next = pool->nodes[0].next;
    pool->nodes[0].next = pos;
    pool->free_count++;
}

/* -------------------------------------------------------------------------- */
/*                           List Operations                                  */
/* -------------------------------------------------------------------------- */

CursorList Cursor_CreateList(CursorPool *pool)
{
    CursorList list = Cursor_Alloc(pool);
    if (list == 0)
    {
        return 0;
    }

    pool->nodes[list].next = 0;
    pool->nodes[list].element = 0;
    return list;
}

void Cursor_ClearList(CursorList list, CursorPool *pool)
{
    if (pool == NULL || list <= 0 || (size_t)list >= pool->capacity)
    {
        return;
    }

    CursorPos p = pool->nodes[list].next;
    pool->nodes[list].next = 0;

    while (p != 0)
    {
        CursorPos temp = pool->nodes[p].next;
        Cursor_Free(p, pool);
        p = temp;
    }
}

void Cursor_DestroyList(CursorList *list_ptr, CursorPool *pool)
{
    if (list_ptr == NULL || *list_ptr == 0)
    {
        return;
    }

    Cursor_ClearList(*list_ptr, pool);
    Cursor_Free(*list_ptr, pool);
    *list_ptr = 0;
}

bool Cursor_IsEmpty(CursorList list, const CursorPool *pool)
{
    if (pool == NULL || list <= 0 || (size_t)list >= pool->capacity)
    {
        return true;
    }
    return (pool->nodes[list].next == 0);
}

bool Cursor_IsLast(CursorPos pos, const CursorPool *pool)
{
    if (pool == NULL || pos <= 0 || (size_t)pos >= pool->capacity)
    {
        return true;
    }
    return (pool->nodes[pos].next == 0);
}

size_t Cursor_Size(CursorList list, const CursorPool *pool)
{
    if (pool == NULL || list <= 0 || (size_t)list >= pool->capacity)
    {
        return 0;
    }

    size_t count = 0;
    CursorPos p = pool->nodes[list].next;

    while (p != 0 && (size_t)p < pool->capacity)
    {
        count++;
        p = pool->nodes[p].next;
    }

    return count;
}

CursorPos Cursor_Find(int value, CursorList list, const CursorPool *pool)
{
    if (pool == NULL || list <= 0 || (size_t)list >= pool->capacity)
    {
        return 0;
    }

    CursorPos p = pool->nodes[list].next;
    while (p != 0 && (size_t)p < pool->capacity)
    {
        if (pool->nodes[p].element == value)
        {
            return p;
        }
        p = pool->nodes[p].next;
    }

    return 0;
}

CursorPos Cursor_FindPrevious(int value, CursorList list, const CursorPool *pool)
{
    if (pool == NULL || list <= 0 || (size_t)list >= pool->capacity)
    {
        return 0;
    }

    CursorPos p = list;
    while (pool->nodes[p].next != 0)
    {
        CursorPos nxt = pool->nodes[p].next;
        if (pool->nodes[nxt].element == value)
        {
            return p;
        }
        p = nxt;
    }

    return p;
}

bool Cursor_Insert(int value, CursorPos pos, CursorPool *pool)
{
    if (pool == NULL || pos < 0 || (size_t)pos >= pool->capacity)
    {
        return false;
    }

    CursorPos tmp = Cursor_Alloc(pool);
    if (tmp == 0)
    {
        return false; /* Out of space */
    }

    pool->nodes[tmp].element = value;
    pool->nodes[tmp].next = pool->nodes[pos].next;
    pool->nodes[pos].next = tmp;
    return true;
}

bool Cursor_InsertFront(int value, CursorList list, CursorPool *pool)
{
    return Cursor_Insert(value, list, pool);
}

bool Cursor_InsertBack(int value, CursorList list, CursorPool *pool)
{
    if (pool == NULL || list <= 0 || (size_t)list >= pool->capacity)
    {
        return false;
    }

    CursorPos p = list;
    while (pool->nodes[p].next != 0)
    {
        p = pool->nodes[p].next;
    }

    return Cursor_Insert(value, p, pool);
}

bool Cursor_Delete(int value, CursorList list, CursorPool *pool)
{
    if (pool == NULL || list <= 0 || (size_t)list >= pool->capacity)
    {
        return false;
    }

    CursorPos p = Cursor_FindPrevious(value, list, pool);
    if (p != 0 && pool->nodes[p].next != 0)
    {
        CursorPos tmp = pool->nodes[p].next;
        if (pool->nodes[tmp].element == value)
        {
            pool->nodes[p].next = pool->nodes[tmp].next;
            Cursor_Free(tmp, pool);
            return true;
        }
    }

    return false;
}

size_t Cursor_ToArray(CursorList list, const CursorPool *pool, int *buffer, size_t max_len)
{
    if (pool == NULL || buffer == NULL || list <= 0 || max_len == 0)
    {
        return 0;
    }

    size_t count = 0;
    CursorPos p = pool->nodes[list].next;

    while (p != 0 && count < max_len)
    {
        buffer[count++] = pool->nodes[p].element;
        p = pool->nodes[p].next;
    }

    return count;
}

void Cursor_PrintList(CursorList list, const CursorPool *pool)
{
    if (pool == NULL || list <= 0 || (size_t)list >= pool->capacity)
    {
        printf("(invalid list)\n");
        return;
    }

    if (Cursor_IsEmpty(list, pool))
    {
        printf("[Header: Slot %d] -> (empty)\n", list);
        return;
    }

    printf("[Header: Slot %d]", list);
    CursorPos p = pool->nodes[list].next;

    while (p != 0)
    {
        printf(" -> [%d (slot %d)]", pool->nodes[p].element, p);
        p = pool->nodes[p].next;
    }

    printf(" -> NULL\n");
}

/* -------------------------------------------------------------------------- */
/*                       Legacy Global API Compatibility                      */
/* -------------------------------------------------------------------------- */

struct node CursorSpace[SPACE_SIZE];

void InitializeCursorSpace(void)
{
    for (int i = 0; i < SPACE_SIZE - 1; i++)
    {
        CursorSpace[i].Element = 0;
        CursorSpace[i].Next = i + 1;
    }
    CursorSpace[SPACE_SIZE - 1].Element = 0;
    CursorSpace[SPACE_SIZE - 1].Next = 0;
}

Position Legacy_CursorAlloc(void)
{
    Position P = CursorSpace[0].Next;
    if (P != 0)
    {
        CursorSpace[0].Next = CursorSpace[P].Next;
        CursorSpace[P].Next = 0;
        CursorSpace[P].Element = 0;
    }
    return P;
}

void Legacy_CursorFree(Position P)
{
    if (P > 0 && P < SPACE_SIZE)
    {
        CursorSpace[P].Element = 0;
        CursorSpace[P].Next = CursorSpace[0].Next;
        CursorSpace[0].Next = P;
    }
}

List Legacy_CursorList(void)
{
    List L = Legacy_CursorAlloc();
    if (L != 0)
    {
        CursorSpace[L].Next = 0;
    }
    return L;
}

List MakeEmpty(List L)
{
    if (L != 0)
    {
        DeleteList(L);
        return L;
    }
    return Legacy_CursorList();
}

int IsEmpty(List L)
{
    return (L > 0 && L < SPACE_SIZE) ? (CursorSpace[L].Next == 0) : 1;
}

int IsLast(Position P, List L)
{
    (void)L;
    return (P > 0 && P < SPACE_SIZE) ? (CursorSpace[P].Next == 0) : 1;
}

void Insert(int X, List L)
{
    Position P = L;
    Position TmpCell = Legacy_CursorAlloc();
    if (TmpCell == 0)
    {
        return;
    }

    while (P > 0 && CursorSpace[P].Next != 0)
    {
        P = CursorSpace[P].Next;
    }

    CursorSpace[TmpCell].Element = X;
    CursorSpace[TmpCell].Next = CursorSpace[P].Next;
    CursorSpace[P].Next = TmpCell;
}

Position Find(int X, List L)
{
    if (L <= 0 || L >= SPACE_SIZE) return 0;
    Position P = CursorSpace[L].Next;
    while (P != 0 && CursorSpace[P].Element != X)
    {
        P = CursorSpace[P].Next;
    }
    return P;
}

Position FindPrevious(int X, List L)
{
    if (L <= 0 || L >= SPACE_SIZE) return 0;
    Position P = L;
    while (CursorSpace[P].Next != 0 &&
           CursorSpace[CursorSpace[P].Next].Element != X)
    {
        P = CursorSpace[P].Next;
    }
    return P;
}

void Delete(int X, List L)
{
    Position P = FindPrevious(X, L);
    if (P > 0 && !IsLast(P, L))
    {
        Position TmpCell = CursorSpace[P].Next;
        CursorSpace[P].Next = CursorSpace[TmpCell].Next;
        Legacy_CursorFree(TmpCell);
    }
}

void DeleteList(List L)
{
    if (L <= 0 || L >= SPACE_SIZE) return;
    Position P = CursorSpace[L].Next;
    CursorSpace[L].Next = 0;
    while (P != 0)
    {
        Position Tmp = CursorSpace[P].Next;
        Legacy_CursorFree(P);
        P = Tmp;
    }
}

void PrintList(List L)
{
    Position P = L;
    printf("\nSlot \t Element  Next\n");
    while (P != 0 && P < SPACE_SIZE)
    {
        printf("% 3d \t % 3d \t % 3d\n", P, CursorSpace[P].Element, CursorSpace[P].Next);
        P = CursorSpace[P].Next;
    }
}
