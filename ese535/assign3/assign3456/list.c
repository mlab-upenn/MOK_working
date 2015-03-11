#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "util.h"
#include "main.h"
#include "globals.h"
#include "tree.h"
#include "asap.h"
#include "domain.h"
#include "part_place.h"

void insert (ListPtr *sPtr, int cell)
{
    // Create a variable of type ListPtr called newPtr, this will store the pointer of the inserted nodes location
    ListPtr newPtr;
    // Allocate memory for the new pointer
    newPtr=malloc( sizeof(ListNode));
    // Guard to make sure memory was actually allocated
    if (newPtr != NULL)
    {
        // Assign cell number to data
        newPtr->cellNumber=cell;
        // Set nextPtr to null temporarily
        newPtr->nextPtr=NULL;
    }
    // Assign the next pointer to the previous address of the start of the list
    newPtr->nextPtr = *sPtr;
    // Assign the pointer to the current node as the start address of the list
    //sPtr = &newPtr;
    *sPtr = newPtr;
}

// Delete a member
void delete(ListPtr *sPtr, int cell)
{
    ListPtr previousPtr;
    ListPtr currentPtr;
    ListPtr tempPtr;

    // We have a very convenient case if we will be deleting the current first node in the list.
    //printf("Trying to delete cell number: %d\n",cell);
    if (cell == (*sPtr)->cellNumber)
    {
        tempPtr=*sPtr;
        *sPtr=(*sPtr)->nextPtr;
        free(tempPtr);
        //printf("Succesfully Deleted, %s\n", block[cell].name);
    }

    // If we have to delete a node from the middle of the list...
    else 
    {
        previousPtr = *sPtr;
        currentPtr = ( *sPtr )-> nextPtr;
        while(currentPtr!=NULL && currentPtr->cellNumber!=cell)
        {
            previousPtr = currentPtr;
            // Walk to next node
            currentPtr = currentPtr->nextPtr;
        }

        if ( currentPtr!=NULL)
        {
            tempPtr = currentPtr;
            previousPtr->nextPtr = currentPtr->nextPtr;
            free(tempPtr);
            //printf("Succesfully Deleted, %s\n", block[cell].name);
        }
        else
        {
            //printf("Couldn't find, %s\n to delete", block[cell].name);
        }
    }
}

// Check if the list is empty
int isEmpty(ListPtr sPtr)
{
    // Reutrns 1 if the the list is empty, 0 otherwise....
    return sPtr==NULL;

}
// Delete an entire list
void freeList(ListPtr *sPtr)
{

}

// Print list
void printList(ListPtr currentPtr)
{
    if(currentPtr == NULL)
    {
        printf("The list is empty\n");
    }
    else
    {
        printf("The list is:\n");
        while (currentPtr != NULL && currentPtr->nextPtr != NULL)
        {
            printf("Block: %s\n", block[currentPtr->cellNumber].name);
            currentPtr=currentPtr->nextPtr;
        }
        printf("\n");
    }
}
