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


//#define DEBUG_PART_PLACE

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
  #ifdef DEBUG_PART_PLACE
  printf("Part size %d used slots %d\n", part_size,tree_location[root].used_slots);
  #endif

  for (i=0;i<tree_location[root].used_slots;i++)
  {
  // First half
    if (i<part_size)
    {
      insert_block_switch(tree_location[root].slots[i],tree_location[root].left,level[tree_location[root].slots[i]]);
      //printf("Slot %d in tree contains block %s\n", i, block[tree_location[tree_location[root].left].slots[i]].name);
  }
  // Second half
    else
      insert_block_switch(tree_location[root].slots[i],tree_location[root].right,level[tree_location[root].slots[i]]);
  }
}

void repartition(int height, int number_of_blocks, int *sub_left, int *sub_right, int location)
{
  int i;
  
  //Naively split blocks into two halves 
  int part_size=number_of_blocks>>1;
  //printf("Part size %d used slots %d\n", part_size,tree_location[location].used_slots);
   // for (i=0;i<number_of_blocks;i++)
    //recurse_left[i]=FALSE;



  for (i=0;i<tree_location[location].used_slots;i++)
  {// First half
    //printf("i equals: %d block %d left child %d\n", i,tree_location[location].slots[i], tree_location[location].left);
    if (i<part_size)
    {
      if (height==1)
      {
        insert_block(tree_location[location].slots[i],tree_location[location].left,level[tree_location[location].slots[i]]);
        //printf("Placed in block %s in PE %d\n", block[tree_location[location].slots[i]].name,tree_location[location].left);
      }
      else
      {
        insert_block_switch(tree_location[location].slots[i],tree_location[location].left,level[tree_location[location].slots[i]]);
      } 
      printf("Left child contains block %s\n", block[tree_location[location].slots[i]].name);
      sub_left[tree_location[location].slots[i]]=1;
      //sub_right[tree_location[location].slots[i]]=0;
      //printf("updated left and right\n");
    }
  // Second half
    else
    {
      if(height==1)
      {
        insert_block(tree_location[location].slots[i],tree_location[location].right,level[tree_location[location].slots[i]]);
        //printf("Placed in block %s in PE %d\n", block[tree_location[location].slots[i]].name,tree_location[location].right) ;      
      }
      else
      {
        insert_block_switch(tree_location[location].slots[i],tree_location[location].right,level[tree_location[location].slots[i]]);
      }
      printf("Right child contains block %s\n", block[tree_location[location].slots[i]].name);
      sub_left[tree_location[location].slots[i]]=0;
      printf("sub_left here is equal to %d\n", sub_left[tree_location[location].slots[i]]);

    }
  }

}

void initialize_F_and_T(int *left)
{
  int i;
  int j;
  int k;
  for(i=0; i<num_nets; i++)
  {
    for(j=0; j<net[i].num_pins; j++)
    {
      int current_block=net[i].pins[j];
      #ifdef DEBUG_PART_PLACE
      printf("Current block: %s\n", block[current_block].name);
      #endif
      if(left[current_block]==1)
      {
        net[i].F_left=(net[i].F_left)+1;
        net[i].T_right=(net[i].T_right)+1;
      }

    else
    { 
      #ifdef DEBUG_PART_PLACE
      printf("Current block: %s caused T_left increase \n", block[current_block].name);
      #endif
      net[i].T_left=net[i].T_left+1;
      net[i].F_right=net[i].F_right+1;
    }
  }
 }
}

void sub_F_and_T(int height, int *sub_left)
{
  int i;
  int j;
  int k;
  for(i=0; i<num_nets; i++)
  {
    for(j=0; j<net[i].num_pins; j++)
    {
      int current_block=net[i].pins[j];
      #ifdef DEBUG_PART_PLACE
      printf("Current block: %s\n", block[current_block].name);
      printf("sub left evals %d\n",sub_left[current_block]);
      #endif
      if(sub_left[current_block]==1)
      {
        #ifdef DEBUG_PART_PLACE
        printf("Current block: %s caused F_left increase \n", block[current_block].name);
        printf("sub left evals %d\n",sub_left[current_block]);
        #endif        
        net[i].F_left=(net[i].F_left)+1;
        net[i].T_right=(net[i].T_right)+1;
      }

    else if(sub_left[current_block]==0)
    { 
      #ifdef DEBUG_PART_PLACE
      printf("Current block: %s caused T_left increase \n", block[current_block].name);
      printf("sub left evals %d\n",sub_left[current_block]);
      #endif
      net[i].T_left=net[i].T_left+1;
      net[i].F_right=net[i].F_right+1;
    }
  }
 }
}


int initialize_left_branch(boolean *is_clock, int *left, int *right, int root, int *gain, int max_gain_left, int pmax, ListPtr * buckets_left, boolean rec)
{
  int i;
  int j;
  int k;
   /* Calculate gain for each block and put in bucket */

  // Populate array "left" with boolean values regarding whether a block is in the left or right branch of the tree
  // Initialize left with all FALSE.  
  if(rec==FALSE)
  {
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
  }
   //max_gain_left=compute_max_gain_left(pmax, buckets_left);
  //printf("Rec was TRUE got here\n");
  //printf("Currently looking at the following tree location %d\n", root);

  for (i=0; i<num_blocks; i++)
  {
    block[i].left=-1;
  }


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
      start=0;
    }

    // For all the nets
    for (j=start; j<numberOfNets; j++)
    {
      #ifdef DEBUG_PART_PLACE
      printf (" j %d, F_left %d number of nets %d\n", j,net[block[current_block].nets[j]].F_left, numberOfNets);
      printf("T_left %d\n", net[block[current_block].nets[j]].T_left);
      #endif

      if(net[block[current_block].nets[j]].F_left==1)
      {
        gain[current_block]=gain[current_block]+1;
        #ifdef DEBUG_PART_PLACE
        printf("gain increased %d\n",gain[current_block]);
        #endif
      }
      else if(net[block[current_block].nets[j]].T_left==0)
      {
        gain[current_block]=gain[current_block]-1;
        #ifdef DEBUG_PART_PLACE
        printf("gain decreased %d\n", gain[current_block]);
        #endif
      }
        
    }
    // Keep track of the maximum gain for indexing the buckets array later...
    #ifdef DEBUG_PART_PLACE
    printf("max gain left %d current gain %d\n", max_gain_left,gain[current_block]);
    #endif
    if (gain[current_block]>= max_gain_left)
    {
      max_gain_left=gain[current_block];
    }
        #ifdef DEBUG_PART_PLACE
    printf("Gain %d on block %s\n", gain[current_block], block[current_block].name);
        #endif

    //blockInfo[current_block].block_address=current_block;
    //blockInfo[current_block].name=block[current_block].name;
    //blockInfo[current_block].gain=gain[current_block];
    //blockInfo[current_block].left=left[i];
    //blockInfo[current_block].free=TRUE;
    block[current_block].gain=gain[current_block];
    block[current_block].left=left[current_block];
    block[current_block].free=1;
  }
  #ifdef DEBUG_PART_PLACE
  printf("LEFT BUCKETS:\n");
  printf("Max gain for left branch: %d\n", max_gain_left);
  #endif
  
  // Fill buckets array
  for (i=0; i<((pmax*2)+1); i++)
  {
    #ifdef DEBUG_PART_PLACE
    printf("i %d, pmax %d\n", i, pmax);
    #endif

    buckets_left[i]=(ListNode*)malloc(sizeof(ListNode));
    buckets_left[i]->nextPtr=NULL;
    buckets_left[i]->cellNumber=-1;
    for (j=0;j<tree_location[tree_location[root].left].used_slots;j++)
    {
     if(block[j].gain==(i-(pmax))&& is_clock[block[j].nets[0]]!=TRUE)
     {

      insert(&buckets_left[i],j);

    }
  } 

  #ifdef DEBUG_PART_PLACE
  printf("Gain: %d\n", (i-(pmax)));
  printList(buckets_left[i]); 
  #endif

 }
 return max_gain_left;

}

int initialize_right_branch(boolean *is_clock, int *left, int *right, int root, int *gain, int max_gain_right, int pmax, ListPtr * buckets_right, boolean rec)
{
  int i;
  int j;
  int k;
 /* RIGHT BRANCH */
  // Populate array "left" with boolean values regarding whether a block is in the left or right branch of the tree
  // Initialize left with all FALSE.  
  if(rec==FALSE)
  {
    for (i=0;i<num_blocks;i++)
    {
      right[i]=FALSE;
    }
  // For all the blocks placed on the right side change left array value to TRUE
    for (i=0;i<tree_location[tree_location[root].right].used_slots;i++)
    {
      right[tree_location[tree_location[root].right].slots[i]]=TRUE;
    #ifdef DEBUG_PART_PLACE
      printf("index %d right %d\n",tree_location[tree_location[root].right].slots[i], right[tree_location[tree_location[root].right].slots[i]]);
    #endif
    }
  }

  // For each block on the right branch
  for (i=0;i<tree_location[tree_location[root].right].used_slots;i++)
  {
    int numberOfNets=0;
    int start = 0;
    int current_block=tree_location[tree_location[root].right].slots[i];
    gain[current_block]=0;
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
    #ifdef DEBUG_PART_PLACE
    printf("Number of nets %d\n", numberOfNets);
    #endif
    // For all the nets
    for (j=start; j<numberOfNets; j++)
    {
      #ifdef DEBUG_PART_PLACE
      printf (" j %d, F_left %d number of nets %d\n", j,net[block[current_block].nets[j]].F_left, numberOfNets);
      printf("T_left %d\n", net[block[current_block].nets[j]].T_left);
      printf("Here\n");
      #endif

      if(net[block[current_block].nets[j]].F_left==1)
      {
        gain[current_block]=gain[current_block]+1;
        #ifdef DEBUG_PART_PLACE
        printf("gain increased %d %dn",gain[current_block],block[current_block].gain);
        #endif
      }
      else if(net[block[current_block].nets[j]].T_left==0)
      {
        gain[current_block]=gain[current_block]-1;
        #ifdef DEBUG_PART_PLACE
        printf("gain decreased %d %d\n", gain[current_block],block[current_block].gain);
        #endif
      }
      
    }
    // Keep track of the maximum gain for indexing the buckets array later...
    if (gain[current_block]>= max_gain_right)
    {
      max_gain_right=gain[current_block];
    }
        #ifdef DEBUG_PART_PLACE
    printf("Gain %d on block %s\n", block[current_block].gain, block[current_block].name);
        #endif

    //blockInfo[current_block].block_address=current_block;
    //blockInfo[current_block].name=block[current_block].name;
    //blockInfo[current_block].gain=gain[current_block];
    //blockInfo[current_block].left=right[i];
    //blockInfo[current_block].free=TRUE;
    //blockInfo[current_block].loc=block[current_block].loc;
    block[current_block].left=right[current_block];
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
      if(block[j].gain==(i-(pmax)) && is_clock[block[j].nets[0]]!=TRUE)
      {
        insert(&buckets_right[i],j);
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
      //printf("Free at last!\n");
}

int which_branch(int max_gain_left, int max_gain_right, int *left, int pmax, int active_branch, ListPtr *buckets_left, ListPtr *buckets_right)
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
    {
      #ifdef DEBUG_PART_PLACE
      printf("max_gain_left %d, pmax %d,Bucket is empty?\n", max_gain_left,pmax);
      #endif
      return -1;
    }
    
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
    printf("Returning left -pmax\n");
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
    printf("Returning right -pmax\n");
    return max_gain_right;
}

int update_gains_left(int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_left, int current_cut)
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
            int current_net=block[current_block_cell].nets[i];
            #ifdef DEBUG_PART_PLACE
            printf("T_left= %d\n", net[current_net].T_left);
            #endif

            // For current net check the T 
            if(net[current_net].T_left==0)
            {
              for(j=0; j<net[block[current_block_cell].nets[i]].num_pins; j++)
              {
                int block_oper=net[block[current_block_cell].nets[i]].pins[j];

                #ifdef DEBUG_PART_PLACE
                printf("looking at pin %d block %s , left = %d on net %d\n", j,block[net[current_net].pins[j]].name,block[block_oper].left, current_net);
                printf("is free: %d, is global clock = %d global clock is %d\n",block[block_oper].free, block[net[j].pins[0]].loc, GLOBAL_CLOCK_LOC);
                #endif 

                

                if(block[block_oper].free==1 &&  block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
                {
                  
                  #ifdef DEBUG_PART_PLACE
                  printf("Left Case 1\n");
                  printf(" T_left =0, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                  #endif

                  block[block_oper].gain=block[block_oper].gain+1;
                  
                  #ifdef DEBUG_PART_PLACE
                  printf(" T_left =0, Updated Gain: %d on block %s\n",block[block_oper].gain, block[block_oper].name);
                  printf("Before Insert\n");
                  //printList(buckets_left[block[block_oper].gain]);
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
          printf("Block moved: %s, left %d\n", block[current_block_cell].name, block[current_block_cell].left);
          //left[current_block_cell]=0;
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
          //printList(buckets_left[max_gain_left+pmax]); 
          printf("End Left Move Active Block\n");
          #endif

    // Update critical nets after the move
         for(i=start; i<numberOfNets; i++)
         {
          int current_net=block[buckets_right[(-max_gain_left)+pmax]->cellNumber].nets[i];
          if(net[current_net].F_left==0)
          {
            for(j=0; j<net[current_net].num_pins; j++)
            {
              int block_oper=net[current_net].pins[j];

              if(block[block_oper].free==1 && block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
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

          else if(net[current_net].F_left==1)
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
                 printf(" Gain +1 F_left =1, Updated Gain: %d, on Block %s, %d\n",block[block_oper].gain,block[block_oper].name,block_oper);
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

int update_gains_right(int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int current_cut)
{
  int i;
  int j;
  int k;
  // If the active block is in the right partition
      if (active_branch==1)
      {
        #ifdef DEBUG_PART_PLACE
        printf("In the right branch");
        #endif
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
          int current_net=block[current_block_cell].nets[i];
          printf("Current Net %d\n",current_net);
        // For current net check the T 
          if(net[current_net].T_right==0)
          {
            for(j=0; j<net[current_net].num_pins; j++)
            {
              int block_oper=net[block[current_block_cell].nets[i]].pins[j];
              printf("block_oper %d current net%d\n", block_oper, current_net);

              if(block[block_oper].free==1 && block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
              {
                //printf("Right Case 1\n");
                #ifdef DEBUG_PART_PLACE
                printf(" T_right =0, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                #endif
                block[block_oper].gain=block[block_oper].gain+1;
                #ifdef DEBUG_PART_PLACE
                printf(" T_right =0, Updated Gain: %d, block %s\n",block[block_oper].gain, block[block_oper].name);
                #endif
                insert(&buckets_right[(block[block_oper].gain)+pmax], block_oper);
                delete(&buckets_right[(block[block_oper].gain)+pmax-1], block_oper);
              }
            }
          }
          else if(net[current_net].T_right==1)
          {
            for(j=0; j<net[current_net].num_pins; j++)
            {
              int block_oper=net[current_net].pins[j];

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
        //left[block_for_move]=1;
        //printf("Block for move is now in right branch\n");
        block[block_for_move].free=0;
        block[block_for_move].gain=-max_gain_right;
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
       printList(buckets_right[max_gain_right+pmax]); 
       #endif
       //printf("We were able to print\n");
       delete(&buckets_right[max_gain_right+pmax], block_for_move);
       //printf("End Left Move Active Block\n");

    // Update critical nets after the move
       for(i=start; i<numberOfNets; i++)
       {
        int current_net=block[buckets_left[(-max_gain_right)+pmax]->cellNumber].nets[i];
        if(net[current_net].F_right==0)
        {
          for(j=0; j<net[current_net].num_pins; j++)
          {
            int block_oper=net[current_net].pins[j];

            if(block[block_oper].free==1 && block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
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
        else if(net[current_net].F_right==1)
        {
          for(j=0; j<net[current_net].num_pins; j++)
          {
            int block_oper=net[current_net].pins[j];

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

void rearrange_blocks(int switch_num)
{
  int i;
  int left_slots_used=tree_location[tree_location[switch_num].left].used_slots;
  int right_slots_used=tree_location[tree_location[switch_num].right].used_slots;
  int left_child=tree_location[tree_location[switch_num].left].location;
  printf("Left child %d, used slots %d\n", left_child, tree_used_slots(left_child));
  int to_delete_left[left_slots_used];
  int to_delete_right[right_slots_used];

  for(i=0; i<left_slots_used; i++)
  {
    to_delete_left[i]=tree_location[tree_location[switch_num].left].slots[i];
  }

  for(i=0; i<right_slots_used; i++)
  {
    to_delete_right[i]=tree_location[tree_location[switch_num].right].slots[i];
  }

  for(i=0; i<left_slots_used; i++)
  {

    //#ifdef DEBUG_PART_PLACE
    printf("removed block in left child %s\n",block[to_delete_left[i]].name);
    //#endif
    // Remove old placemnt in subtree
    remove_block(to_delete_left[i], tree_location[switch_num].left);
  }

  for (i=0;i<right_slots_used;i++)
  {
    // #ifdef DEBUG_PART_PLACE
    printf("removed block in right child %s\n",block[to_delete_right[i]].name);
    //#endif
    // Remove old placemnt in subtree
    remove_block(to_delete_right[i], tree_location[switch_num].right);
  }

  printf("Should be empty:\n");

  for(i=0; i<tree_used_slots(left_child); i++)
  {
    printf("Should be empty %s\n",block[tree_location[tree_location[switch_num].left].slots[i]].name);
  }

  //#ifdef DEBUG_PART_PLACE
  printf("Done removing blocks\n");
  //#endif
  for(i=0; i<num_blocks; i++)
  {
    printf("Block %s left = %d\n", block[i].name, block[i].left);
  }

  for(i=0; i<num_blocks;i++)
  {
    //left[i]=FALSE;
    /// Skip placing clock, but mark it so doesn't complain later
    //if(block[i].loc == GLOBAL_CLOCK_LOC || block[i].type==-1)
    if(block[i].loc == GLOBAL_CLOCK_LOC)
      {
        //#ifdef DEBUG_PART_PLACE
        printf("Skipped placing block %s, it is a global clock\n", block[i].name);
        //#endif

        block[i].loc=GLOBAL_CLOCK_LOC;
        block[i].slot_loc=GLOBAL_CLOCK_LOC;
      }
    // If block is left, then place in a slot in the left child.
    else if(block[i].left==1)
    {
      printf("Node type %d\n", tree_location[tree_location[switch_num].left].node_type);
      if (tree_location[tree_location[switch_num].left].node_type==1)
      {
        place_block(i,tree_location[switch_num].left,level[i]);
        printf("Placed block %s in PE %d\n", block[i].name,tree_location[switch_num].left);
      }
      else
      {
        insert_block_switch(i, tree_location[switch_num].left,level[i]);
      }

        //#ifdef DEBUG_PART_PLACE
        printf("Placed block %s in left child\n",block[i].name);
        printf("Used slots left %d\n", tree_location[tree_location[switch_num].left].used_slots);
        //#endif 
      }
    // If block is right, then place in a slot in the right child
    else if(block[i].left==0)
      {

      if (tree_location[tree_location[switch_num].right].node_type==1)
      {
        place_block(i,tree_location[switch_num].right,level[i]);
        printf("Placed block %s in PE %d\n", block[i].name,tree_location[switch_num].right);
      }
      else
      {
        insert_block_switch(i, tree_location[switch_num].right,level[i]);
      }

        //#ifdef DEBUG_PART_PLACE
        printf("Placed block %s in right child\n",block[i].name);
        printf("Used slots right %d\n", tree_location[tree_location[switch_num].right].used_slots);
        //#endif
      }
      
  }
}

int rec_initialize_left_branch(int *left, int *right, int tree_loc, int max_gain_left, int pmax, ListPtr * buckets_left)
{
  int i;
  int j;
  int k;
   
  //printf("Currently looking at the following tree location %d\n", tree_loc);

  // Set all the left/right status indicators to -1
  // meaning that they are not placed at this location
  for (i=0; i<num_blocks; i++)
  {
    block[i].left=-1;
    block[i].gain=0;
  }


  // Loop through all of the blocks which are present in the tree structure's left child
  for (i=0;i<tree_location[tree_location[tree_loc].left].used_slots;i++)
  { 
    int numberOfNets=0;
    int start = 0;
    int current_block=tree_location[tree_location[tree_loc].left].slots[i];

    #ifdef DEBUG_PART_PLACE
    printf("Current Block: %s\n", block[current_block].name);
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
      start=0;
    }

    // For all the nets
    for (j=start; j<numberOfNets; j++)
    {
      #ifdef DEBUG_PART_PLACE
      printf (" j %d, F_left %d number of nets %d\n", j,net[block[current_block].nets[j]].F_left, numberOfNets);
      printf("T_left %d\n", net[block[current_block].nets[j]].T_left);
      #endif

      if(net[block[current_block].nets[j]].F_left==1)
      {
        block[current_block].gain=block[current_block].gain+1;
        #ifdef DEBUG_PART_PLACE
        printf("gain increased %d\n",gain[current_block]);
        #endif
      }
      else if(net[block[current_block].nets[j]].T_left==0)
      {
        block[current_block].gain=block[current_block].gain-1;
        #ifdef DEBUG_PART_PLACE
        printf("gain decreased %d\n", gain[current_block]);
        #endif
      }
        
    }

    // Keep track of the maximum gain for indexing the buckets array later...
    #ifdef DEBUG_PART_PLACE
    printf("max gain left %d current gain %d\n", max_gain_left,gain[current_block]);
    #endif

    if (block[current_block].gain >= max_gain_left)
    {
      max_gain_left=block[current_block].gain;
    }

    #ifdef DEBUG_PART_PLACE
    printf("Gain %d on block %s\n", gain[current_block], block[current_block].name);
    #endif

    //block[current_block].gain=gain[current_block];
    //printf("evaluation of left %d on block %s\n",left[current_block], block[current_block].name);
    block[current_block].left=left[current_block];
    block[current_block].free=1;
  }
  #ifdef DEBUG_PART_PLACE
  printf("LEFT BUCKETS:\n");
  printf("Max gain for left branch: %d\n", max_gain_left);
  #endif
  
  // Fill buckets array
  for (i=0; i<((pmax*2)+1); i++)
  {
    #ifdef DEBUG_PART_PLACE
    printf("i %d, pmax %d\n", i, pmax);
    #endif

    buckets_left[i]=(ListNode*)malloc(sizeof(ListNode));
    buckets_left[i]->nextPtr=NULL;
    buckets_left[i]->cellNumber=-1;

    for (j=0;j<tree_location[tree_location[tree_loc].left].used_slots;j++)
    {
     int l_index=tree_location[tree_location[tree_loc].left].slots[j];
     #ifdef DEBUG_PART_PLACE
     printf("Got here l_index is equal to: %s\n", block[l_index].name);
     #endif
     if(block[l_index].gain==(i-(pmax)) && block[l_index].loc != GLOBAL_CLOCK_LOC)
     {

      insert(&buckets_left[i],l_index);
    }
  } 

  #ifdef DEBUG_PART_PLACE
  printf("Gain: %d\n", (i-(pmax)));
  printList(buckets_left[i]); 
  #endif

 }
 return max_gain_left;

}

int rec_initialize_right_branch(int *left, int *right, int tree_loc, int max_gain_right, int pmax, ListPtr * buckets_right)
{
  int i;
  int j;
  int k;
 
  // For each block on the right branch
  for (i=0;i<tree_location[tree_location[tree_loc].right].used_slots;i++)
  {
    int numberOfNets=0;
    int start = 0;
    int current_block=tree_location[tree_location[tree_loc].right].slots[i];

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
      start=0;
    }

    #ifdef DEBUG_PART_PLACE
    printf("Number of nets %d\n", numberOfNets);
    #endif

    // For all the nets
    for (j=start; j<numberOfNets; j++)
    {
      #ifdef DEBUG_PART_PLACE
      printf (" j %d, F_left %d number of nets %d\n", j,net[block[current_block].nets[j]].F_left, numberOfNets);
      printf("T_left %d\n", net[block[current_block].nets[j]].T_left);
      printf("Here\n");
      #endif

      if(net[block[current_block].nets[j]].F_left==1)
      {
        block[current_block].gain=block[current_block].gain+1;
        #ifdef DEBUG_PART_PLACE
        printf("gain increased %d %dn",gain[current_block],block[current_block].gain);
        #endif
      }
      else if(net[block[current_block].nets[j]].T_left==0)
      {
        block[current_block].gain=block[current_block].gain-1;
        #ifdef DEBUG_PART_PLACE
        printf("gain decreased %d %d\n", gain[current_block],block[current_block].gain);
        #endif
      }
      
    }
    // Keep track of the maximum gain for indexing the buckets array later...
    if (block[current_block].gain>= max_gain_right)
    {
      max_gain_right=block[current_block].gain;
    }
    
    #ifdef DEBUG_PART_PLACE
    printf("Gain %d on block %s\n", block[current_block].gain, block[current_block].name);
    #endif

    printf("Evaluation of left %d on current block %s\n", left[current_block], block[current_block].name);
    block[current_block].left=left[current_block];
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

    for (j=0;j<tree_location[tree_location[tree_loc].right].used_slots;j++)
    {
      int l_index=tree_location[tree_location[tree_loc].right].slots[j];
      #ifdef DEBUG_PART_PLACE
      printf("Got here l_index is equal to: %s\n", block[l_index].name);
      #endif
      if(block[l_index].gain==(i-(pmax)) && block[l_index].loc != GLOBAL_CLOCK_LOC)
      {
        insert(&buckets_right[i],l_index);
      }

    } 
    #ifdef DEBUG_PART_PLACE
    printf("Gain: %d\n", (i-(pmax)));
    printList(buckets_right[i]); 
    #endif
  }
  return max_gain_right;
}

int rec_KLFM(int current_height, struct tree_node *left_switch, struct tree_node *right_switch, int pmax, boolean *is_clock, int location)
{

    printf("KLFM CALL ON LOCATION %d\n", location);
    // Local variables
    int i;
    int l_max_gain_left;
    int l_max_gain_right;
    int current_cut;
    int previous_cut=current_cut+1;
    int passes=0;
    int active_branch;

    // Figure out how many blocks in this switch
    int number_of_blocks=tree_location[location].used_slots;

    // Initialize Gain array
    int *l_gain = (int *)malloc(num_blocks*sizeof(int));
    for (i=0; i<num_blocks; i++)
      l_gain[i]=0;

    // Intialize buckets
    ListPtr *buckets_left;
    buckets_left=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

    ListPtr *buckets_right;
    buckets_right=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

    // Initialize left array with -1 signifying index is not in this switch
    int *sub_left=(int *)malloc(sizeof(int)*num_blocks);
    for (i=0; i<num_blocks;i++)
    {
      block[i].left=-1;
      sub_left[i]=-1;
    }

    // Initialize left array with -1 signifying index is not in this switch
    int *sub_right=(int *)malloc(sizeof(int)*num_blocks);
    for (i=0; i<num_blocks;i++)
    {
      block[i].left=-1;
      sub_right[i]=-1;
    }

    // Initial partition
    repartition(current_height, number_of_blocks, sub_left, sub_right, location);

    // Recompute F and T's
    sub_F_and_T(current_height, sub_left);

    // Max gains
    l_max_gain_left=-pmax;
    l_max_gain_right=-pmax;
    l_max_gain_left=rec_initialize_left_branch(sub_left, sub_right, location, l_max_gain_left, pmax, buckets_left);

    printf("Parents blocks %d\n", location);
    for(i=0; i<tree_used_slots(location); i++)
    {
      printf(" block %s, left= %d\n",block[tree_location[location].slots[i]].name, sub_left[tree_location[location].slots[i]]);
    }

    printf("Left child blocks location %d\n", tree_location[location].left);
    for(i=0; i<tree_used_slots(tree_location[location].left); i++)
    {
      printf(" block %s\n",block[tree_location[tree_location[location].left].slots[i]].name);
    }

    printf("Right child blocks location %d\n", tree_location[location].right);
    for(i=0; i<tree_used_slots(tree_location[location].right); i++)
    {
      printf(" block %s\n",block[tree_location[tree_location[location].right].slots[i]].name);
    }

    l_max_gain_right=rec_initialize_right_branch(sub_left, sub_right, location, l_max_gain_right, pmax, buckets_right);


    //Current cut
    current_cut=calculate_cut(sub_left);

    /* KLFM */
    while(current_cut<previous_cut && passes<10 && current_cut>0)
    {
      passes=passes+1;

      #ifdef DEBUG_PART_PLACE 
      printf("Pass Number: %d\n",passes);
      #endif

      free_blocks();
      int locked_nodes=0;

      while(locked_nodes<(num_blocks-1)&& current_cut>0 && ((l_max_gain_right>=0 && l_max_gain_right>= l_max_gain_left) || ( l_max_gain_left>=0 && l_max_gain_left>= l_max_gain_right)))
      {
    // Find Max Gain Left
        l_max_gain_left=compute_max_gain_left(pmax, buckets_left);

        #ifdef DEBUG_PART_PLACE
        printf("max gain left: %d\n",l_max_gain_left);
        #endif

    // Find Max Gain Right
        l_max_gain_right=compute_max_gain_right(pmax, buckets_right);

        #ifdef DEBUG_PART_PLACE
        printf("max gain right: %d\n",l_max_gain_right);
        #endif

    // Select active branch
        active_branch=which_branch(l_max_gain_left,l_max_gain_right,sub_left, pmax, active_branch, buckets_left, buckets_right);
        if(active_branch==-1)
          break;
        previous_cut=current_cut;

    // If active branch is left
        current_cut=update_gains_left(active_branch, buckets_left, buckets_right, pmax, l_max_gain_left, current_cut);


    // If active branch is right
        current_cut=update_gains_right(active_branch, buckets_left, buckets_right, pmax, l_max_gain_right, current_cut);

        //#ifdef DEBUG_PART_PLACE
        printf("Current Cut after update %d\n", current_cut);
        //#endif

    // Add 1 to locked nodes count
        locked_nodes=locked_nodes+1;
      }


      //free(sub_left);
    }
    return(current_cut);

}

void recurse_tree(int height, int location, int pmax, boolean *is_clock)
{
  int i;
  if(height==0)
  {
    printf("Location %d called recurse, but it is a PE\n", location);
    return;
  }
  else
  {
    rec_KLFM(height, tree_location[location].left, tree_location[location].right, pmax, is_clock, location);
    printf("Parents blocks = %d\n",tree_used_slots(location));
    printf("Left blocks = %d\n", tree_used_slots(tree_location[location].left));
    printf("Right blocks= %d\n",tree_used_slots(tree_location[location].right));
    printf("BEGIN REARRANGE \n");
    rearrange_blocks(location);
    printf("END REARRANGE BLOCKS\n");
    printf("NEW ROUND\n");
    recurse_tree(height-1, tree_location[location].left, pmax, is_clock);
    recurse_tree(height-1, tree_location[location].right, pmax, is_clock);
    return;
    
  }
}



