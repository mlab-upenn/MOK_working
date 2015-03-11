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
#include <string.h>


#define DEBUG_PART_PLACE

int compute_pmax(void)
{
  int i;
  int pmax_local;
  pmax_local=-num_blocks;
  for (i=0; i<num_nets;i++)
  {
    if (net[i].num_pins>pmax_local)
    {
      pmax_local=net[i].num_pins;
    }
  }
  return pmax_local;
}

void initial_partition(int root, int cluster_size)
{
  int i;
  //Naively split blocks into two halves 
  int part_size=(get_size()*cluster_size)>>1;

  for (i=0;i<tree_location[root].used_slots;i++)
  // First half
    if (i<part_size)
      insert_block_switch(tree_location[root].slots[i],tree_location[root].left,level[tree_location[root].slots[i]]);
  // Second half
    else
      insert_block_switch(tree_location[root].slots[i],tree_location[root].right,level[tree_location[root].slots[i]]);
}

void initialize_F_and_T(boolean *left)
{
  int i;
  int j;
  int k;
  for(i=0; i<num_nets; i++)
  {
    if(mixed_net(net[i],left) == TRUE)
    {
      for(j=1; j<net[i].num_pins; j++)
      {
        int current_block=net[i].pins[j];
        if(block[current_block].left==TRUE)
        {
          net[i].F_left=(net[i].F_left)+1;
          net[i].T_right=(net[i].T_right)+1;
        }

      }
    }

      else if (block[current_block].left==FALSE)
      {
        net[i].T_left=net[i].T_left+1;
        net[i].F_right=net[i].F_right+1;
      }
    }
  }
}

int initialize_left_branch(boolean *is_clock, int *left, int *right, int root, int *gain, int count, int max_gain_left, struct block_info *blockInfo, int pmax, ListPtr * buckets_left)
{
  int i;
  int j;
  int k;
  /* Calculate gain for each block and put in bucket */

  // Populate array "left" with boolean values regarding whether a block is in the left or right branch of the tree
  // Initialize left with all FALSE.  
  for (i=0;i<num_blocks;i++)
  {
    left[i]=FALSE;
  }

  // For all the blocks placed on the left side change left array value to TRUE
  for (i=0;i<tree_location[tree_location[root].left].used_slots;i++)
  {
    left[tree_location[tree_location[root].left].slots[i]]=TRUE;
  }

  initialize_F_and_T(left);

  // For each block on the left branch
  for (i=0;i<tree_location[tree_location[root].left].used_slots;i++)
  { 
    int numberOfNets=0;
    int start = 0;
    int current_block=tree_location[tree_location[root].left].slots[i];

    #ifdef DEBUG_PART_PLACE
    printf("Current Block: %d\n", current_block);
    #endif

    // Figure out how many nets are connected to the block
    if(block[current_block].type==1 || block[current_block].type==3)
    {
      numberOfNets=block[current_block].num_nets-2;
      start=1;
    }
    else
    {
      numberOfNets=block[current_block].num_nets;
    }

    // For all the nets
    for (j=start; j<numberOfNets; j++)
    {
      printf (" j %d, F_left %d number of nets %d\n", j,net[block[current_block].nets[j]].F_left, numberOfNets);
      printf("T_left %d\n", net[block[current_block].nets[j]].T_left);

      if(net[block[current_block].nets[j]].F_left==1)
      {
        gain[current_block]=gain[current_block]+1;
      }
      else if(net[block[current_block].nets[j]].T_left==0)
      {
        gain[current_block]=gain[current_block]-1;
      }
        
    }
    // Keep track of the maximum gain for indexing the buckets array later...
    if (gain[i]>= max_gain_left)
    {
      max_gain_left=gain[i];
    }
        #ifdef DEBUG_PART_PLACE
    printf("Gain %d on block %s\n", gain[i], block[current_block].name);
        #endif

    blockInfo[current_block].block_address=current_block;
    blockInfo[current_block].name=block[current_block].name;
    blockInfo[current_block].gain=gain[i];
    blockInfo[current_block].left=left[i];
    blockInfo[current_block].free=TRUE;
    block[current_block].gain=gain[i];
    block[current_block].left=left[i];
    block[current_block].free=1;
  }
      #ifdef DEBUG_PART_PLACE
  printf("LEFT BUCKETS:\n");
  printf("Max gain for left branch: %d\n", max_gain_left);
      #endif
    // Fill buckets array
  for (i=0; i<((pmax*2)+1); i++)
  {
    printf("i %d, pmax %d\n", i, pmax);
    buckets_left[i]=(ListNode*)malloc(sizeof(ListNode));
    buckets_left[i]->nextPtr=NULL;
    buckets_left[i]->cellNumber=-1;
    for (j=0;j<tree_location[tree_location[root].left].used_slots+1;j++)
    {
     if(blockInfo[j].gain==(i-(pmax))&& is_clock[block[j].nets[0]]!=TRUE)
     {

      insert(&buckets_left[i],blockInfo[j].block_address);

    }
  } 
    //#ifdef DEBUG_PART_PLACE
  printf("Gain: %d\n", (i-(pmax)));
  printList(buckets_left[i]); 
    //#endif

 }
 return max_gain_left;

}

int initialize_right_branch(boolean *is_clock, int *left, int *right, int root, int *gain, int count, int max_gain_right, struct block_info *blockInfo, int pmax, ListPtr * buckets_right)
{
  int i;
  int j;
  int k;
 /* RIGHT BRANCH */
  // Populate array "left" with boolean values regarding whether a block is in the left or right branch of the tree
  // Initialize left with all FALSE.  
  for (i=0;i<num_blocks;i++)
  {
    right[i]=FALSE;
  }
  // For all the blocks placed on the right side change left array value to TRUE
  for (i=0;i<tree_location[tree_location[root].right].used_slots;i++)
  {
    right[tree_location[tree_location[root].right].slots[i]]=TRUE;
    printf("index %d right %d\n",tree_location[tree_location[root].right].slots[i], right[tree_location[tree_location[root].right].slots[i]]);
  }

  // For each block on the right branch
  for (i=0;i<tree_location[tree_location[root].right].used_slots;i++)
  {
    int numberOfNets=0;
    int start = 0;
    int current_block=tree_location[tree_location[root].right].slots[i];
    #ifdef DEBUG_PART_PLACE
    printf("Current Block: %d\n", current_block);
    #endif

    // Figure out how many nets are connected to the block
    if(block[current_block].type==1 || block[current_block].type==3)
    {
      numberOfNets=block[current_block].num_nets-2;
      start=1;
    }
    else
    {
      numberOfNets=block[current_block].num_nets;
    }
    printf("Number of nets %d\n", numberOfNets);
    // For all the nets
    for (j=start; j<numberOfNets; j++)
    {
      // If the net has a pin on both sides of the partition
      //printf(" right eval %d net %d\n", right[net[block[current_block].nets[j]].pins[i]],j);

        // If the net has a pin on both sides of the partition
        if(mixed_net(block[current_block].nets[j],right) == TRUE)
        {
        // Initialize count to zero
          count=0;
        // For each pins on the net
          for (k=1; k<net[j].num_pins; k++)
          {
          // If the block associated with the pin is on the left hand side
            if (right[net[j].pins[k]]==TRUE)
            {
            // Increase the count
              count=count+1;
              //net[block[current_block].nets[j]].F_right=count;
            }
          }
        // If there is not more than one pin with a block on the left hand side
          if (count<=1)
          {
        // Increment the gain
            gain[current_block]=gain[current_block]+1;
            printf("gain plus one gain: %d\n", gain[current_block]);
          }
        }

      // If this is not a mixed net decrement the gain (moving this block relative to this net will make things worse)
        else 
        {
          gain[current_block]=gain[current_block]-1;
          printf("gain minus one");
          //net[block[current_block].nets[j]].T_right=numberOfNets-start;
        }
      
    }
    // Keep track of the maximum gain for indexing the buckets array later...
    if (gain[current_block]>= max_gain_right)
    {
      max_gain_right=gain[current_block];
    }
        #ifdef DEBUG_PART_PLACE
    printf("Gain %d on block %s\n", gain[current_block], block[current_block].name);
        #endif

    blockInfo[current_block].block_address=current_block;
    blockInfo[current_block].name=block[current_block].name;
    blockInfo[current_block].gain=gain[current_block];
    blockInfo[current_block].left=right[i];
    blockInfo[current_block].free=TRUE;
    blockInfo[current_block].loc=block[current_block].loc;
    block[current_block].gain=gain[current_block];
    block[current_block].free=1;

  }
      #ifdef DEBUG_PART_PLACE
  printf("RIGHT BUCKETS:\n");
  printf("Max gain for right branch: %d\n", max_gain_right);
      #endif

    // Fill buckets array
  for (i=0; i<((pmax*2)+1); i++)
  {
    buckets_right[i]=(ListNode*)malloc(sizeof(ListNode));
    buckets_right[i]->nextPtr=NULL;
    buckets_right[i]->cellNumber=-1;

    for (j=(num_blocks-tree_location[tree_location[root].right].used_slots);j<num_blocks;j++)
    {
      if(blockInfo[j].gain==(i-(pmax)) && is_clock[block[j].nets[0]]!=TRUE)
      {
        insert(&buckets_right[i],blockInfo[j].block_address);
      }

    } 
    #ifdef DEBUG_PART_PLACE
    printf("Gain: %d\n", (i-(pmax)));
    printList(buckets_right[i]); 
    #endif
  }
  return max_gain_right;
}
void free_blocks(void)
{
  int i;
     for (i=0; i<num_blocks; i++)
       {
        block[i].free=1;
      }
}

int which_branch(int max_gain_right, int max_gain_left, int *left, int pmax, int active_branch, ListPtr *buckets_left, ListPtr *buckets_right)
{
  int i;
  int left_count=0;
  int right_count=0;

  for (i=0; i<num_blocks;i++)
  {
    #ifdef DEBUG_PART_PLACE
    printf("Block: %s, i= %d, is free: %d, gain is: %d, is left? %d\n", block[i].name,i, block[i].free, block[i].gain, block[i].left);
    #endif

    if(block[i].loc != GLOBAL_CLOCK_LOC && block[i].type!=-1)
    {
          if(block[i].left==1)
          left_count=left_count+1;
          if(block[i].left==0)
          right_count=right_count+1;

    }
  }
    

  float balance=0;

  if((right_count+left_count)!=0)
  {
    balance=(float)left_count/(float)(right_count+left_count);
  }

  float r=.4;
  int smax=5;

  #ifdef DEBUG_PART_PLACE
  printf("Balance: %f, r %f\n",balance, r);
  printf("Left count %d, Right count %d\n", left_count,right_count);
  #endif


  if( (r*(right_count+left_count)-smax) <= left_count && (r*(right_count+left_count)+smax) >=left_count)
  {

    if (max_gain_right>max_gain_left)
    {
      active_branch=1;
              #ifdef DEBUG_PART_PLACE
      printf("Right Branch Active\n");
              #endif
    }
    else
    {
      active_branch=0;
              #ifdef DEBUG_PART_PLACE
      printf("Left Branch Active\n");
              #endif
    }
  }

  else if(balance<r)
  {
    active_branch=1;
    if(buckets_right[max_gain_right+pmax]->cellNumber==-1)
    return -1;
    
    #ifdef DEBUG_PART_PLACE
    printf("By balance Right Branch Active\n");
    printf("cellNumber %d\n", buckets_right[max_gain_right+pmax]->cellNumber);
    #endif
  }
  else if(balance>=r)
  {
    active_branch=0;
    if(buckets_left[max_gain_left+pmax]->cellNumber==-1)
    return -1;
    
    #ifdef DEBUG_PART_PLACE
    printf("By balance Left Branch Active\n");
    printf("cellNumber %d\n",buckets_left[max_gain_left+pmax]->cellNumber);
    #endif
  }
 // Determine active block (for debugging purposes)
  if(active_branch==0)
  {
    #ifdef DEBUG_PART_PLACE
    printf("Max Gain: %d, Bucket: %d\n", max_gain_left, (max_gain_left)+pmax);
    printf("Active: %s\n",block[buckets_left[(max_gain_left)+pmax]->cellNumber].name);
    #endif
  }
  else
  {
    #ifdef DEBUG_PART_PLACE
    printf("Max Gain: %d, Bucket: %d\n", max_gain_right, (max_gain_right)+pmax);
    printf("Active: %s\n",block[buckets_right[(max_gain_right)+pmax]->cellNumber].name);
    #endif
  }
  //printf("max gain right %d max gain left %d\n",max_gain_right, max_gain_left);
  return active_branch;
}

int compute_max_gain_left(int pmax, ListPtr *buckets_left)
{
 int max_gain_left=-pmax;
  int i;
    for(i=(pmax*2);i>-1;i--)
    {
      int whichBlock=buckets_left[i]->cellNumber;
      //printf("For loop iteration %d cell number %d\n", i, whichBlock);
      //printList(buckets_left[i]);
      if (whichBlock>=0 && block[whichBlock].free==1)
      {
        //printf("Max gain right (i):%d\n", i);
        max_gain_left=i-pmax;
        return max_gain_left;
      }
    }
    //printf("Returning -pmax\n");
    return max_gain_left;
}

int compute_max_gain_right(int pmax,ListPtr *buckets_right)
{
  int max_gain_right=-pmax;
  int i;
    for(i=(pmax*2);i>-1;i--)
    {
      int whichBlock=buckets_right[i]->cellNumber;
      //printList(buckets_right[i]);
      //printf("For loop iteration %d cell number %d\n", i, whichBlock);
      if (whichBlock>=0 && block[whichBlock].free==1)
      {
        //printf("Max gain right (i):%d\n", i);
        max_gain_right=i-pmax;
        return max_gain_right;
      }
    }
    printf("Returning -pmax\n");
    return max_gain_right;
}

int update_gains_left(int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_left, boolean *left, int current_cut)
{
  int i;
  int j;
  int k;

  if (active_branch==0)
        {

        // Housekeeping
          int current_block_cell=buckets_left[(max_gain_left)+pmax]->cellNumber;
          int start=0;
          int numberOfNets=0;
          block[current_block_cell].free=0;

        
        // Figure out how many nets are connected to the block
          if(block[current_block_cell].type==1 || block[current_block_cell].type==3)
          {
            numberOfNets=block[current_block_cell].num_nets-2;
            start=1;
          }
          else
          {
            numberOfNets=block[current_block_cell].num_nets;
            start=0;
          }

         // Before move check the nets in the "to block" ie any nets with a pin in the right half
          for(i=start; i<numberOfNets; i++)
          {
            // Using index i, pick up the current net:
            int current_net=block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i];
            #ifdef DEBUG_PART_PLACE
            printf("T_left= %d\n", net[current_net].T_left);
            #endif
            // For current net check the T 
            if(net[current_net].T_left==0)
            {
              for(j=0; j<net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].num_pins; j++)
              {
                int block_oper=net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].pins[j];

                #ifdef DEBUG_PART_PLACE
                printf("looking at pin %d block %s , left = %d on net %d\n", j,block[net[current_net].pins[j]].name,block[block_oper].left, current_net);
                printf("is free: %d, is global clock = %d global clock is %d\n",block[block_oper].free, block[net[j].pins[0]].loc, GLOBAL_CLOCK_LOC);
                #endif 

                

                if(block[block_oper].left==1 && block[block_oper].free==1 &&  block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
                {
                  
                  #ifdef DEBUG_PART_PLACE
                  printf("Left Case 1\n");
                  printf(" T_left =0, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                  #endif

                  block[block_oper].gain=block[block_oper].gain+1;
                  
                  #ifdef DEBUG_PART_PLACE
                  printf(" T_left =0, Updated Gain: %d\n",block[block_oper].gain);
                  printf("Before Insert\n");
                  printList(buckets_left[block[block_oper].gain]);
                  #endif

                  insert(&buckets_left[(block[block_oper].gain+pmax)], block_oper);
                  delete(&buckets_left[block[block_oper].gain+pmax-1], block_oper);
                  
                  #ifdef DEBUG_PART_PLACE 
                  printf("After Delete\n");
                  printf("Completed Left Case 1\n");
                  #endif
                  
                }
              }
            }
            else if(net[current_net].T_left==1)
            {
              for(j=0; j<net[current_net].num_pins; j++)
              {
                int block_oper=net[current_net].pins[j];

                if(block[block_oper].left==0 && block[block_oper].free==1)
                {
                  
                  #ifdef DEBUG_PART_PLACE
                  printf("Left Case 2\n");
                  printf(" T_left =1, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                  #endif

                  block[block_oper].gain=block[block_oper].gain-1;

                  #ifdef DEBUG_PART_PLACE
                  printf(" Gain-1, T_left =1, Updated Gain: %d, on Block %s, %d\n",block[block_oper].gain,block[block_oper].name,block_oper);
                  #endif

                  insert(&buckets_right[(block[block_oper].gain)+pmax], block_oper);
                  delete(&buckets_right[(block[block_oper].gain)+pmax+1], block_oper);

                  #ifdef DEBUG_PART_PLACE
                  printf("Completed Left Case 2\n");
                  #endif

                }
              }
            }
          }

        // Move Block and update F and T for each net.
          #ifdef DEBUG_PART_PLACE
          printf("Begin Left Move Active Block\n");
          #endif

          //int block_for_move=buckets_left[(max_gain_left)+pmax]->cellNumber;
          // Move base cells and 
          block[current_block_cell].left=0;
          left[current_block_cell]=0;
          block[current_block_cell].free=0;
          block[current_block_cell].gain=-max_gain_left;
          insert(&buckets_right[-max_gain_left+pmax], current_block_cell);
      


          #ifdef DEBUG_PART_PLACE
          printf("max_gain_left %d pmax %d\n",max_gain_left,pmax);
          printf("Found block for move\n");
          #endif



          for(i=start; i<numberOfNets; i++)
          {
            int current_net=block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i];
              net[current_net].T_left = net[current_net].T_left+1;
              net[current_net].F_right=net[current_net].T_left;
              net[current_net].F_left = net[current_net].F_left-1;
              net[current_net].T_right=net[current_net].F_left;
          }

          #ifdef DEBUG_PART_PLACE
          printf("Block for move index %d, name : %s, gain: %d\n", current_block_cell, block[current_block_cell].name, block[current_block_cell].gain);
          printList(buckets_left[max_gain_left+pmax]); 
          #endif
          
          delete(&buckets_left[max_gain_left+pmax], current_block_cell);
         

          #ifdef DEBUG_PART_PLACE
          printf("New list with deletion\n");
          printList(buckets_left[max_gain_left+pmax]); 
          printf("End Left Move Active Block\n");
          #endif

    // Update critical nets after the move
         for(i=start; i<numberOfNets; i++)
         {
          int current_net=block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i];
          if(net[current_net].F_left==0)
          {
            for(j=0; j<net[current_net].num_pins; j++)
            {
              int block_oper=net[current_net].pins[j];

              if(block[block_oper].left==0 && block[block_oper].free==1 && block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
              {
                
                #ifdef DEBUG_PART_PLACE
                //printf("Left Case 3\n");
                printf(" F_left =0, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                #endif

                block[block_oper].gain=block[block_oper].gain-1;

                #ifdef DEBUG_PART_PLACE
                printf(" Gain -1 F_left =0, Updated Gain: %d, on Block %s, %d\n",block[block_oper].gain,block[block_oper].name,block_oper);
                #endif

                insert(&buckets_right[(block[block_oper].gain)+pmax ], block_oper);
                delete(&buckets_right[(block[block_oper].gain)+pmax+1], block_oper);
              }
            }
          }

          else if(net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].F_left==1)
          {
            for(j=0; j<net[current_net].num_pins; j++)
            {
              int block_oper=net[current_net].pins[j];

              if(block[block_oper].left==1 && block[block_oper].free==1)
              {

                #ifdef DEBUG_PART_PLACE
                //printf("Left Case 4\n");                
                printf(" F_left =1, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                #endif

                block[block_oper].gain=block[block_oper].gain+1;
                
                #ifdef DEBUG_PART_PLACE
                printf(" F_left =1, Updated Gain: %d\n",block[block_oper].gain);
                #endif

                insert(&buckets_left[(block[block_oper].gain)+pmax], block_oper);
                delete(&buckets_left[(block[block_oper].gain)+pmax-1], block_oper);
                //printf("Completed Left Case 4\n");
              }
            }
          }
        }
        return (current_cut-max_gain_left);
      }
      return current_cut; 
    }

int update_gains_right(int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, boolean *left, int current_cut)
{
  int i;
  int j;
  int k;
  // If the active block is in the right partition
      if (active_branch==1)
      {
        printf("In the right branch");
        // Housekeeping
        int current_block_cell=buckets_right[(max_gain_right)+pmax]->cellNumber;
        int start=0;
        int numberOfNets=0;
        block[current_block_cell].free=0;

        // Figure out how many nets are connected to the block
        if(block[current_block_cell].type==1 || block[current_block_cell].type==3)
        {
          numberOfNets=block[current_block_cell].num_nets-2;
          start=1;
        }
        else
        {
          numberOfNets=block[current_block_cell].num_nets;
        }
      // Before move check the nets in the "to block" ie any nets with a pin in the right half
        for(i=start; i<numberOfNets; i++)
        {
        // Using index i, pick up the current net:
          int current_net=block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i];
        // For current net check the T 
          if(net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].T_right==0)
          {
            for(j=0; j<net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].num_pins; j++)
            {
              int block_oper=net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].pins[j];

              if(block[block_oper].left==0 && block[block_oper].free==1 && block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
              {
                //printf("Right Case 1\n");
                  #ifdef DEBUG_PART_PLACE
                printf(" T_right =0, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                  #endif
                block[block_oper].gain=block[block_oper].gain+1;
                  #ifdef DEBUG_PART_PLACE
                printf(" T_right =0, Updated Gain: %d\n",block[block_oper].gain);
                  #endif
                insert(&buckets_right[(block[block_oper].gain)+pmax], block_oper);
                delete(&buckets_right[(block[block_oper].gain)+pmax-1], block_oper);
              }
            }
          }
          else if(net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].T_right==1)
          {
            for(j=0; j<net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].num_pins; j++)
            {
              int block_oper=net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].pins[j];

              if(block[block_oper].left==1 && block[block_oper].free==1)
              {
                //printf("Right Case 2\n");
                  #ifdef DEBUG_PART_PLACE
                printf(" T_right =1, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                  #endif
                block[block_oper].gain=block[block_oper].gain-1;
                  #ifdef DEBUG_PART_PLACE
                printf(" T_right =1, Updated Gain: %d\n",block[block_oper].gain);
                  #endif
                insert(&buckets_left[(block[block_oper].gain)+pmax], block_oper);
                delete(&buckets_left[(block[block_oper].gain)+pmax+1], block_oper);
              }
            }
          }
        }

    // Move Block and update F and T for each net.
        //printf("Begin Right Move Active Block\n");
        int block_for_move=buckets_right[(max_gain_right)+pmax]->cellNumber;
        //printf("Found block for move\n");
        block[block_for_move].left=1;
        left[block_for_move]=1;
        //printf("Block for move is now in right branch\n");
        block[block_for_move].free=0;
        //printf("Block for move is locked\n");
        insert(&buckets_left[-max_gain_right+pmax], block_for_move);
        //printf("Block for move is placed in buckets right\n");
        for(i=start; i<numberOfNets; i++)
        {

         int current_net=block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i];
         net[current_net].T_right = net[current_net].T_right+1;
         net[current_net].F_left = net[current_net].T_right;
         net[current_net].F_right = net[current_net].F_right-1;
         net[current_net].T_left = net[current_net].F_right;
        }

       #ifdef DEBUG_PART_PLACE
       printf("Block for move index %d, name : %s, gain: %d\n", block_for_move, block[block_for_move].name, block[block_for_move].gain);
       #endif
       printList(buckets_right[max_gain_right+pmax]); 
       //printf("We were able to print\n");
       delete(&buckets_right[max_gain_right+pmax], block_for_move);
       //printf("End Left Move Active Block\n");

    // Update critical nets after the move
       for(i=start; i<numberOfNets; i++)
       {
        if(net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].F_right==0)
        {
          for(j=0; j<net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].num_pins; j++)
          {
            int block_oper=net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].pins[j];

            if(block[block_oper].left==1 && block[block_oper].free==1 && block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
            {
              //printf("Right case 3\n");
                #ifdef DEBUG_PART_PLACE
              printf(" F_right =0, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                #endif
              block[block_oper].gain=block[block_oper].gain-1;
                #ifdef DEBUG_PART_PLACE
              printf(" F_right =0, Updated Gain: %d\n",block[block_oper].gain);
                #endif
              insert(&buckets_left[(block[block_oper].gain)+pmax], block_oper);
              delete(&buckets_left[(block[block_oper].gain)+pmax+1], block_oper);
            }
          }
        }
        else if(net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].F_right==1)
        {
          for(j=0; j<net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].num_pins; j++)
          {
            int block_oper=net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].pins[j];

            if(block[block_oper].left==0 && block[block_oper].free==1)
            {
              //printf("Right case 4\n");
                #ifdef DEBUG_PART_PLACE
              printf(" F_right =0, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                #endif
              block[block_oper].gain=block[block_oper].gain+1;
                #ifdef DEBUG_PART_PLACE
              printf(" F_right =1, Updated Gain: %d\n",block[block_oper].gain);
                #endif
              insert(&buckets_right[(block[block_oper].gain)+pmax], block_oper);
              delete(&buckets_right[(block[block_oper].gain)+pmax-1], block_oper);
            }
          }
        }
      }
      return (current_cut-max_gain_right);
    }
    return current_cut;
}