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
#include "updategains.h"
#include <string.h>


//#define DEBUG_PART_PLACE
//#define CHECK_GAINS

// Helpers for later

void print_f_and_t(void)
{
  int i;
   for(i=0; i<num_nets; i++)
 {
  printf("F_left %d T_left %d net %d\n", net[i].F_left, net[i].T_left, i);
  printf("F_right %d T_right %d net %d\n", net[i].F_right, net[i].T_right, i);
 }
}
int array_scan(int whichElem, int *array, int num_elem)
{
  int i;
  for(i=0;i<num_elem;i++)
  {
    if(array[i]==whichElem)
      return 0;
  }
  return 1;
}

void check_gains(int blockLoc)
{
  int real;
  real = calculate_gains(blockLoc);
  if(real!=block[blockLoc].gain)
  {
    printf("ERROR \n \n \n");
    printf("Gain %d on block %s does not match calculation %d\n", block[blockLoc].gain, block[blockLoc].name, real);


  }
  else
    printf("Successful update\n");
}

int calculate_gains(int blockLoc)
{
  int i;
  int j;
  int gain=0;
  int numberOfNets;
  int start;
  int to_left=0;
  int to_right=0;
  int from_left=0;
  int from_right=0;

  for(i=0; i<num_nets; i++)
  {
    for(j=0; j<net[i].num_pins; j++)
    {
      int current_block=net[i].pins[j];
      #ifdef DEBUG_PART_PLACE
      printf("Current block: %s\n", block[current_block].name);
      printf("sub left evals %d\n",sub_left[current_block]);
      #endif
      if(block[current_block].left==1)
      {
        #ifdef DEBUG_PART_PLACE
        printf("Current block: %s caused F_left increase \n", block[current_block].name);
        printf("sub left evals %d\n",sub_left[current_block]);
        #endif    
        from_left=from_left+1;
        to_right=to_right+1;
      }

    else if(block[current_block].left==0)
    { 
      #ifdef DEBUG_PART_PLACE
      printf("Current block: %s caused T_left increase \n", block[current_block].name);
      printf("sub left evals %d\n",sub_left[current_block]);
      #endif
      to_left=to_left+1;
      from_right=from_right+1;

    }
  }
 }

    if(block[blockLoc].type==1 || block[blockLoc].type==3)
  {
    numberOfNets=block[blockLoc].num_nets-2;
    start=0;
  }
  else
  {
    numberOfNets=block[blockLoc].num_nets;
    start=0;
  }

  for (i=start; i<numberOfNets; i++)
  {
    printf("F_left %d F_right %d T_left %d T_right %d net %d current gain %d\n", net[i].F_left, net[i].F_right, net[i].T_left, net[i].T_right, i, block[blockLoc].gain);
    if((from_left==1 && block[blockLoc].left==1)||(from_right==1 && block[blockLoc].left==0))
    {
      gain=gain+1;
    }
    else if((to_left=0 && block[blockLoc].left==1)||(to_right==0 && block[blockLoc].left==0))
    {
      gain=gain-1;
    }
  }

  return gain;
}

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

// KLFM stuff in order
void repartition(int height, int number_of_blocks, int *sub_left, int *sub_right, int location)
{
  int i;
  
  //Naively split blocks into two halves 
  int part_size=number_of_blocks>>1;

  for (i=0;i<tree_location[location].used_slots;i++)
  {
    if (i<part_size)
    {
      if (height==1)
      {
        insert_block(tree_location[location].slots[i],tree_location[location].left,level[tree_location[location].slots[i]]);
        
      }
      else
      {
        insert_block_switch(tree_location[location].slots[i],tree_location[location].left,level[tree_location[location].slots[i]]);
        //printf("Placed in block %s in %d\n", block[tree_location[location].slots[i]].name,tree_location[location].left);
      } 
      #ifdef DEBUG_PART_PLACE
      printf("Left child contains block %s\n", block[tree_location[location].slots[i]].name);
      #endif
      sub_left[tree_location[location].slots[i]]=1;

    }
  // Second half
    else
    {
      if(height==1)
      {
        insert_block(tree_location[location].slots[i],tree_location[location].right,level[tree_location[location].slots[i]]); 
      }
      else
      {
        insert_block_switch(tree_location[location].slots[i],tree_location[location].right,level[tree_location[location].slots[i]]);
        //printf("Placed in block %s in %d\n", block[tree_location[location].slots[i]].name,tree_location[location].right);
      }
      #ifdef DEBUG_PART_PLACE
      printf("Right child contains block %s\n", block[tree_location[location].slots[i]].name);
      #endif
      sub_left[tree_location[location].slots[i]]=0;
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

int rec_initialize_left_branch(int *left, int *right, int tree_loc, int max_gain_left, int pmax, ListPtr * buckets_left)
{
  int i;
  int j;
  int k;
   
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
    if(block[current_block].type==LATCH || block[current_block].type==LUT_AND_LATCH)
    {
      numberOfNets=block[current_block].num_nets-1;
      start=0;
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
     //#ifdef DEBUG_PART_PLACE
     //printf("Got here l_index is equal to: %s gain %d\n", block[l_index].name, block[l_index].gain);
     //#endif
     if(block[l_index].gain==(i-(pmax)) && block[l_index].loc != GLOBAL_CLOCK_LOC)
     {

      insert(&buckets_left[i],l_index);
      //printf("inserted a block in buckets left\n");
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
    printf(" Initialize right branch current Block: %s\n", block[current_block].name);
    #endif

    // Figure out how many nets are connected to the block
    if(block[current_block].type==1 || block[current_block].type==3)
    {
      numberOfNets=block[current_block].num_nets-1;
      start=0;
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

      if(net[block[current_block].nets[j]].F_right==1)
      {
        block[current_block].gain=block[current_block].gain+1;
        #ifdef DEBUG_PART_PLACE
        printf("gain increased %d %dn",gain[current_block],block[current_block].gain);
        #endif
      }
      else if(net[block[current_block].nets[j]].T_right==0)
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

    //printf("Evaluation of left %d on current block %s\n", left[current_block], block[current_block].name);
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

int compute_max_gain_left(int pmax, ListPtr *buckets_left)
{
 int max_gain_left=-pmax-1;
  int i;
    for(i=(pmax*2);i>-1;i--)
    {
      int whichBlock=buckets_left[i]->cellNumber;
      //printf("For loop iteration %d cell number %d\n", i, whichBlock);
      //printList(buckets_left[i]);
      if (whichBlock>=0 && block[whichBlock].free==1)
      {
        //printf("Max gain left (i):%d\n", i);
        max_gain_left=i-pmax;
        return max_gain_left;
      }
    }
    printf("Returning left -pmax\n");
    return max_gain_left;
}

int compute_max_gain_right(int pmax,ListPtr *buckets_right)
{
  int max_gain_right=-pmax-1;
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

void free_blocks(void)
{
  int i;
     for (i=0; i<num_blocks; i++)
       {
        block[i].free=1;
      }
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

    if(block[i].loc != GLOBAL_CLOCK_LOC)
    {
          if(block[i].left==1)
          {//printf("In left: %s\n", block[i].name);
          left_count=left_count+1;
        }
          if(block[i].left==0)
          right_count=right_count+1;

    }
  }
    

  float balance=0;

  if((right_count+left_count)!=0)
  {
    balance=(float)left_count/(float)(right_count+left_count);
  }

  float r=.5;
  int smax=5;

  #ifdef DEBUG_PART_PLACE
  printf("Balance: %f, r %f\n",balance, r);
  printf("Left count %d, Right count %d\n", left_count,right_count);
  #endif


  if( ((r*((float)right_count+(float)left_count)-(float)smax) <= (float)left_count) && ((r*((float)right_count+(float)left_count)+(float)smax) >= (float)left_count))
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
    printf("Balance was less than r\n");
    active_branch=1;
    if(max_gain_right < -pmax)
    {
      printf("But max gain right was less than pmax\n");
      return -1;
    }

    if(buckets_right[max_gain_right+pmax]->cellNumber==-1)
    {
      printf("But we are looking at an empty bucket?\n");
      return -1;
    }
    
    #ifdef DEBUG_PART_PLACE
    printf("By balance Right Branch Active\n");
    printf("cellNumber %d\n", buckets_right[max_gain_right+pmax]->cellNumber);
    #endif
  }
  else if(balance>=r)
  {
    active_branch=0;
    if(max_gain_left < -pmax)
      return -1;
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

int KLFM(int current_height,  int pmax, boolean *is_clock, int location)
{
  // Housekeeping
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
  l_max_gain_left=-pmax-1;
  l_max_gain_right=-pmax-1;
  l_max_gain_left=rec_initialize_left_branch(sub_left, sub_right, location, l_max_gain_left, pmax, buckets_left);
  l_max_gain_right=rec_initialize_right_branch(sub_left, sub_right, location, l_max_gain_right, pmax, buckets_right);
  current_cut=calculate_cut(sub_left);
  printf("Current Cut after initialization: %d\n", current_cut);

  /* KLFM */
  while(current_cut<=previous_cut && passes<10)
  {
    passes=passes+1;
    free_blocks();
    int locked_nodes=0;

    #ifdef DEBUG_PART_PLACE 
    printf("Pass Number: %d\n",passes);
    #endif

    while(locked_nodes<(num_blocks)&& ((l_max_gain_right>=0 && l_max_gain_right>= l_max_gain_left) || ( l_max_gain_left>=0 && l_max_gain_left>= l_max_gain_right)))
    {

      // Find Max Gain Left
      l_max_gain_left=compute_max_gain_left(pmax, buckets_left);
      
      #ifdef DEBUG_PART_PLACE
      printf("max gain left: %d\n",l_max_gain_left);
      for (i=0; i<((pmax*2)+1); i++)
      {
        printf("Gain: %d\n", (i-(pmax)));
        printList(buckets_left[i]); 
      }
      #endif

      // Find Max Gain Right
      l_max_gain_right=compute_max_gain_right(pmax, buckets_right);

      if(l_max_gain_right<-pmax && l_max_gain_left<-pmax)
        break;
    
        

      #ifdef DEBUG_PART_PLACE
      printf("max gain right: %d\n",l_max_gain_right);
      for (i=0; i<((pmax*2)+1); i++)
      {
        printf("Gain: %d\n", (i-(pmax)));
        printList(buckets_left[i]); 
      }        
      #endif

      // Select active branch
      active_branch=which_branch(l_max_gain_left,l_max_gain_right,sub_left, pmax, active_branch, buckets_left, buckets_right);
      if(active_branch==-1)
        break;

      previous_cut=current_cut;

      printf("ACTIVE BRANCH %d PMAX %d l_max_gain_right %d l_max_gain_left %d\n", active_branch, pmax, l_max_gain_right, l_max_gain_left);

      // Update gains
      current_cut=update_gains(active_branch, buckets_left, buckets_right, pmax, l_max_gain_left, l_max_gain_right, current_cut);

      #ifdef DEBUG_PART_PLACE
      printf("Current Cut after update %d\n", current_cut);
      #endif

      // Add 1 to locked nodes count
      locked_nodes=locked_nodes+1;
    }
  }
  return(current_cut);
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

    #ifdef DEBUG_PART_PLACE
    printf("removed block in left child %s\n",block[to_delete_left[i]].name);
    #endif
    // Remove old placemnt in subtree
    remove_block(to_delete_left[i], tree_location[switch_num].left);
  }

  for (i=0;i<right_slots_used;i++)
  {
    #ifdef DEBUG_PART_PLACE
    printf("removed block in right child %s\n",block[to_delete_right[i]].name);
    #endif
    // Remove old placemnt in subtree
    remove_block(to_delete_right[i], tree_location[switch_num].right);
  }

  printf("Should be empty:\n");

  for(i=0; i<tree_used_slots(left_child); i++)
  {
    printf("Should be empty %s\n",block[tree_location[tree_location[switch_num].left].slots[i]].name);
  }

  #ifdef DEBUG_PART_PLACE
  printf("Done removing blocks\n");
  #endif
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
        #ifdef DEBUG_PART_PLACE
        printf("Skipped placing block %s, it is a global clock\n", block[i].name);
        #endif

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

        #ifdef DEBUG_PART_PLACE
        printf("Placed block %s in left child\n",block[i].name);
        printf("Used slots left %d\n", tree_location[tree_location[switch_num].left].used_slots);
        #endif 
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

        #ifdef DEBUG_PART_PLACE
        printf("Placed block %s in right child\n",block[i].name);
        printf("Used slots right %d\n", tree_location[tree_location[switch_num].right].used_slots);
        #endif
      }
      
  }
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
    KLFM(height, pmax, is_clock, location);
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


