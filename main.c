#include <stdio.h>
#include <stdlib.h>
#define SPACE_SIZE 11

typedef int List;
typedef int Position;
struct node
{
    int Element;
    int Next;
};

struct node CursorSpace[SPACE_SIZE];

void InitializeCursorSpace();
List CursorList();
Position CursorAlloc();
List MakeEmpty(List);
int IsEmpty(List);
void Insert(int, List);
void Insert(int, List);
Position Find(int, List);
Position FindPrevious(int, List);
void Delete(int, List);
void CursorFree(Position);
void DeleteList(List);
void PrintList(List);

int main()
{

    InitializeCursorSpace();

    List myList1 = CursorAlloc();
    Insert(1, myList1);
    Insert(2, myList1);
    Insert(3, myList1);

    List myList2 = CursorAlloc();
    Insert(11, myList2);
    Insert(22, myList2);
    Insert(33, myList2);

    PrintList(myList1);
    PrintList(myList2);

    printf("Done!\n");
    return 0;
}

void InitializeCursorSpace()
{
    int i;
    for (i = 0; i < SPACE_SIZE - 1; i++)
    {
        CursorSpace[i].Next = i + 1;
    }
    CursorSpace[SPACE_SIZE - 1].Next = 0;
}

List CursorList()
{
    List L = CursorAlloc();
    CursorSpace[L].Next = 0;
    return L;
}

Position CursorAlloc()
{
    Position P;
    // get the position of the first position in the free list
    P = CursorSpace[0].Next;
    CursorSpace[0].Next = CursorSpace[P].Next;
    if (P == 0)
    {
        printf("Out of space !\n");
        exit(0);
    }
    return P;
}

// empties the list that is sent here (the header of the list)
List MakeEmpty(List L)
{
    if (L)
        DeleteList(L);
    L = CursorAlloc();
    if (L == 0)
        printf("Out of memory!");
    CursorSpace[L].Next = 0;
    return L;
}

// returns true if the list is empty
int IsEmpty(List L)
{
    return CursorSpace[L].Next == 0; // header refers to zero
}

// Return true if P is the last position in list L
/* Parameter L is unused in this implementation */
int IsLast(Position P, List L)
{
    return CursorSpace[P].Next == 0;
}

// Insert (after legal position P)
void Insert(int X, List L)
{
    Position TmpCell, P = L;
    TmpCell = CursorAlloc(); // will allocate the second node (after the header) in the array.It also makes sure the CursorSpace[0] to point to a new empty location
    while (P && CursorSpace[P].Next != 0)
        P = CursorSpace[P].Next;
    CursorSpace[TmpCell].Element = X;
    CursorSpace[TmpCell].Next = CursorSpace[P].Next;
    CursorSpace[P].Next = TmpCell;
}

// Return Position of X in L; 0 if not found
Position Find(int X, List L)
{
    Position P;
    P = CursorSpace[L].Next;
    while (P && CursorSpace[P].Element != X)
        P = CursorSpace[P].Next;
    return P;
}

// If X is not found, then Next field of returned value is 0
Position FindPrevious(int X, List L)
{
    Position P;
    P = L;
    while (CursorSpace[P].Next &&
           CursorSpace[CursorSpace[P].Next].Element != X)
        P = CursorSpace[P].Next;
    return P;
}

/* Assume that the position is legal */
void Delete(int X, List L)
{
    Position P, TmpCell;
    P = FindPrevious(X, L);
    if (!IsLast(P, L))
    {
        TmpCell = CursorSpace[P].Next;
        CursorSpace[P].Next = CursorSpace[TmpCell].Next;
        CursorFree(TmpCell);
    }
}

void CursorFree(Position P)
{
    CursorSpace[P].Next = CursorSpace[0].Next;
    CursorSpace[0].Next = P;
}

void DeleteList(List L)
{
    Position P, Tmp;
    P = CursorSpace[L].Next; /* Header assumed */
    CursorSpace[L].Next = 0;
    while (P != 0)
    {
        Tmp = CursorSpace[P].Next;
        CursorFree(P);
        P = Tmp;
    }
}

void PrintList(List L)
{
    Position P = L;
    printf("\nSlot \t Element  Next\n");
    while (P != 0)
    {
        printf("% 3d \t % 3d \t % 3d\n", P, CursorSpace[P].Element, CursorSpace[P].Next);
        P = CursorSpace[P].Next;
    }
}
