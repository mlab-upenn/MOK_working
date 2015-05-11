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
        #ifdef DEBUG_PART_PLACE
        printf("assigning cell number %d\n", cell);
        #endif
        // Assign cell number to data
        newPtr->cellNumber=cell;
        // Set nextPtr to null temporarily
        newPtr->nextPtr=NULL;
    }
    else
    {
        printf("There was a problem with the pointer!\n");
    }
    // Assign the next pointer to the previous address of the start of the list
    newPtr->nextPtr = *sPtr;
    // Assign the pointer to the current node as the start address of the list
    //sPtr = &newPtr;
    *sPtr = newPtr;
}

void insert_path (ListPtr *sPtr, int location, int detail_loc, int local_timestep)
{
    // Create a variable of type ListPtr called newPtr, this will store the pointer of the inserted nodes location
    ListPtr newPtr;
    ListPtr currentPtr;
    // Allocate memory for the new pointer
    newPtr=(ListPtr)malloc( sizeof(ListNode));
    // Guard to make sure memory was actually allocated
    if (newPtr != NULL)
    {
        newPtr->location=location;
        newPtr->detail_loc=detail_loc;
        newPtr->local_timestep=local_timestep;
        newPtr->nextPtr=NULL;
    }
    else
    {
        printf("There was a problem with the pointer!\n");
    }
    // Assign the next pointer to the previous address of the start of the list
    currentPtr=*sPtr;
    newPtr->nextPtr = currentPtr;
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
            #ifdef DEBUG_PART_PLACE
            printf("Succesfully Deleted, %s\n", block[cell].name);
            #endif
        }
        else
        {
            #ifdef DEBUG_PART_PLACE
            printf("Couldn't find, %s\n to delete", block[cell].name);
            #endif
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
    ListPtr currentPtr;
    ListPtr delPtr;
    printf("about to assign currentPtr\n");
    delPtr=*sPtr;
    while(delPtr != NULL)
    {
        currentPtr= delPtr->nextPtr;
        printf("About to free tempPtr\n");
        if(delPtr != NULL)
        free(delPtr);
        delPtr=currentPtr;  
        printf("freed tempPtr\n");
    }
    printf("exited\n");
    sPtr=NULL; 
    printf("returning to previous function\n");
    return;          
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
