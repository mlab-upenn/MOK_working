// Create global data structure for a list
extern struct listNode *leftBranch;
extern struct listNode *rightBranch;
extern struct listNode *freeCells;

struct listNode{
    int cellNumber;
    int gain;
    struct listNode *nextPtr;
};

// Define List type and and pointer to a List.
typedef struct listNode ListNode;
typedef ListNode *ListPtr;

// Functions for manipulating lists
// Insert a new member
void insert (ListPtr *sPtr, int cellNumber);
// Delete a member
void delete(ListPtr *sPtr, int cellNumber);
// Check if the list is empty
int isEmpty(ListPtr sPtr);
// Delete an entire list
void freeList(ListPtr *sPtr);

void printList(ListPtr currentPtr);

