#pragma once
// Create global data structure for a list
extern struct listNode *leftBranch;
extern struct listNode *rightBranch;
extern struct listNode *freeCells;
extern struct listNode *routePath;

struct listNode{
    int cellNumber;
    int gain;
    int placement;
    int location;
    int detail_loc;
    int local_timestep;
    struct listNode *nextPtr;
};


// Define List type and and pointer to a List.
typedef struct listNode ListNode;
typedef ListNode *ListPtr;

// Functions for manipulating lists
// Insert a new member
void insert (ListPtr *sPtr, int cellNumber);
void insert_path (ListPtr *sPtr, int location, int detail_loc, int local_timestep);
// Delete a member
void delete(ListPtr *sPtr, int cellNumber);
// Check if the list is empty
int isEmpty(ListPtr sPtr);
// Delete an entire list
void freeList(ListPtr *sPtr);

void printList(ListPtr currentPtr);

