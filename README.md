# Cursor-Based Linked List in C (Array-Based Memory Pool)

[![CI](https://github.com/i7modes/CursorLinkedLists/actions/workflows/ci.yml/badge.svg)](https://github.com/i7modes/CursorLinkedLists/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![C Standard](https://img.shields.io/badge/C-C99-blue.svg)
![Memory Status](https://img.shields.io/badge/Valgrind-0%20Leaks-brightgreen.svg)

A clean, robust, and educational implementation of a **Cursor-Based Linked List** in C99. Implements dynamic memory allocation (`malloc` and `free`) and pointers inside a pre-allocated array memory arena using a **freelist**.

---

## Architecture: Array Memory Pool & Freelist

In environments without pointers or dynamic heap allocation (such as safety-critical embedded systems), linked lists can be simulated within a static or contiguous array.

```text
                  +-------------------------------------------------------------+
                  |                     Memory Arena Buffer                     |
+---------------+---------------+---------------+---------------+---------------+
| Slot 0 (Free) | Slot 1 (Head) | Slot 2 (Data) | Slot 3 (Data) | Slot 4 (Free) |
| Next -> 4     | Next -> 2     | Next -> 3     | Next -> 0     | Next -> 0     |
+---------------+---------------+---------------+---------------+---------------+
        |               |                                               ^
        |               +========> [1] -> [2] -> [3] -> NULL            |
        +---------------------------------------------------------------+
```

### How it Works:
1. **Slot 0 is the Freelist Head**: It points to the first unallocated slot.
2. **`Cursor_Alloc()` (Simulating `malloc`)**: Detaches the first available slot from the freelist in $\mathcal{O}(1)$ time.
3. **`Cursor_Free()` (Simulating `free`)**: Prepends a released slot back onto the freelist in $\mathcal{O}(1)$ time.
4. **Multiple Lists**: Multiple independent linked lists can coexist seamlessly inside the same memory arena.

---

## Features

- **Freelist Memory Management**: Strict $\mathcal{O}(1)$ slot allocation and deallocation without system heap fragmentation.
- **Multiple Independent Lists**: Create and manage multiple disjoint lists inside one shared pool.
- **Crash-Safe Error Handling**: Safely returns `0` (null cursor position) when the memory pool is exhausted without aborting the program.
- **Memory Diagnostic Dump (`CursorPool_PrintDump`)**: Visualizes slot index, data payload, next pointer, and allocation status (Free vs In-Use).
- **Automated Unit Testing & CI**: 38 unit test assertions covering allocation exhaustion, slot recycling, and multi-list coexistence.
- **Legacy Compatibility**: Full support for classic Weiss-style global signatures (`InitializeCursorSpace`, `CursorAlloc`, `CursorFree`, `Insert`, `Delete`, `Find`, etc.).

---

## Complexity Analysis

| Operation | Time Complexity | Space Complexity | Description |
| :--- | :---: | :---: | :--- |
| `Cursor_Alloc` | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | Detaches slot from freelist |
| `Cursor_Free` | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | Returns slot to freelist |
| `Cursor_CreateList` | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | Allocates list header slot |
| `Cursor_Insert` | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | Inserts value after specified cursor slot |
| `Cursor_InsertBack` | $\mathcal{O}(n)$ | $\mathcal{O}(1)$ | Traverses to end of list and inserts |
| `Cursor_Delete` | $\mathcal{O}(n)$ | $\mathcal{O}(1)$ | Finds value and recycles slot to freelist |
| `Cursor_Find` | $\mathcal{O}(n)$ | $\mathcal{O}(1)$ | Linear scan through list slots |
| `Cursor_Size` | $\mathcal{O}(n)$ | $\mathcal{O}(1)$ | Counts nodes in list |
| `CursorPool_Destroy` | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | Frees entire arena buffer |

---

## Project Structure

```text
CursorLinkedLists/
├── include/
│   └── cursor_list.h            # Public API, pool structs, and legacy aliases
├── src/
│   └── cursor_list.c            # Freelist allocator and cursor list implementation
├── examples/
│   └── demo.c                   # Terminal showcase with visual memory arena dumps
├── tests/
│   └── test_cursor_list.c       # Automated unit test suite (38 assertions)
├── .github/
│   └── workflows/
│       └── ci.yml               # GitHub Actions CI matrix
├── Makefile                     # Cross-platform build script
├── LICENSE                      # MIT License
├── .gitignore                   # Ignore binaries and temporary files
└── README.md                    # Project documentation
```

---

## Quick Start

### 1. Clone the Repository
```bash
git clone https://github.com/i7modes/CursorLinkedLists.git
cd CursorLinkedLists
```

### 2. Build and Run Demo
```bash
make demo
make run-demo
```

### 3. Run Automated Unit Tests
```bash
make check
```

Expected output:
```text
========================================
  Cursor Linked List Automated Tests
========================================

--- Test: Pool Creation & Allocation Safety ---
  [PASS] CursorPool_Create(5) returns non-NULL
  [PASS] Capacity is 5
  [PASS] Free slots count is 4 (slot 0 reserved)
  [PASS] Successfully allocated 4 slots
...
========================================
  Results: 38/38 assertions passed
========================================
```

### 4. Run Memory Leak Check (Linux / WSL)
```bash
make test
valgrind --leak-check=full --show-leak-kinds=all ./bin/test_runner
```

---

## Code Example

```c
#include "cursor_list.h"
#include <stdio.h>

int main(void) {
    // 1. Create a memory pool of 10 slots
    CursorPool *pool = CursorPool_Create(10);

    // 2. Allocate two independent lists
    CursorList list1 = Cursor_CreateList(pool);
    CursorList list2 = Cursor_CreateList(pool);

    // 3. Insert items into lists
    Cursor_InsertBack(10, list1, pool);
    Cursor_InsertBack(20, list1, pool);
    Cursor_InsertBack(100, list2, pool);

    // 4. Print lists
    Cursor_PrintList(list1, pool); // [Header: Slot 1] -> [10 (slot 3)] -> [20 (slot 4)] -> NULL
    Cursor_PrintList(list2, pool); // [Header: Slot 2] -> [100 (slot 5)] -> NULL

    // 5. Inspect memory arena
    CursorPool_PrintDump(pool);

    // 6. Clean up
    CursorPool_Destroy(&pool); // 0 memory leaks!
    return 0;
}
```

---

## License

Distributed under the [MIT License](file:///c:/Users/i7mod/OneDrive/GitHub/CursorLinkedLists/LICENSE). Created by [i7modes](https://github.com/i7modes).
