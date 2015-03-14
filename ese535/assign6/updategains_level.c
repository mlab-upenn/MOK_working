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

int update_gains(int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_left, int max_gain_right, int current_cut)
{
  int i;
  int j;
  int k;
  int T;
  int F;

  // Housekeeping
  int start=0;
  int numberOfNets=0;
  int whichBlock;

  // Which block to move
  if(active_branch==0)
  {
  whichBlock=buckets_left[(max_gain_left)+pmax]->cellNumber;
  block[whichBlock].free=0;
  }
  else if (active_branch==1)
  {
    #ifdef DEBUG_PART_PLACE
    printf("Max gain right: %d\n", max_gain_right);
    printf("Block %s\n", block[buckets_right[(max_gain_right)+pmax]->cellNumber].name);
    #endif
  whichBlock=buckets_right[(max_gain_right)+pmax]->cellNumber;
  block[whichBlock].free=0;    
  }


  // Figure out how many nets are connected to the block
  if(block[whichBlock].type==LATCH || block[whichBlock].type==LUT_AND_LATCH)
  {
  numberOfNets=block[whichBlock].num_nets-1;
  start=0;
  }

  else
  {
  numberOfNets=block[whichBlock].num_nets;
  start=0;
  }

  // Setup arrays to make sure we don't include nets and pins twice in an update
  int markedNets[MAXLUT+2];
  for(i=0; i<MAXLUT+2;i++)
  markedNets[i]=-1;

  // Before move check the nets in the "to block" 
  for(i=start; i<numberOfNets; i++)
  {
  // Using index i, pick up the current net:
  int whichNet=block[whichBlock].nets[i];
  if(array_scan(whichNet,markedNets, MAXLUT+2)==0)
  {
  #ifdef DEBUG_PART_PLACE
  printf("Duplicate Net!\n");
  #endif
  continue;
  }

  // Place net in markedNets array
  markedNets[i]=whichNet;


  // Check for critical nets
  update_case_1(whichBlock, active_branch, whichNet, buckets_left, buckets_right, pmax, max_gain_right, max_gain_left);

  update_case_2(whichBlock, active_branch, whichNet, buckets_left, buckets_right, pmax, max_gain_right, max_gain_left);
  }

  // Move Block and update F and T for each net.
  update_case_move(active_branch, whichBlock, buckets_left, buckets_right, pmax, max_gain_right, max_gain_left, start, numberOfNets);

  // Setup arrays to make sure we don't include nets and pins twice in an update
  for(i=0; i<MAXLUT+2;i++)
  markedNets[i]=-1;

  // Update critical nets after the move
  for(i=0; i<numberOfNets; i++)
  {
    int whichNet;
    
    // Using index i, pick up the current net:
    whichNet=block[whichBlock].nets[i];
    //printf("which net b: %d\n", whichNet);
    
    if(array_scan(whichNet,markedNets,MAXLUT+2)==0)
    {
    //printf("Duplicate Net!\n");
    continue;
  }
    
    // Place net in markedNets array
    markedNets[i]=whichNet;
    
    update_case_3(whichBlock, active_branch, whichNet, buckets_left, buckets_right, pmax, max_gain_right, max_gain_left);
    update_case_4(whichBlock, active_branch, whichNet, buckets_left, buckets_right, pmax, max_gain_right, max_gain_left);
  }

  if(active_branch==0)
  {
    return (current_cut-max_gain_left);
  }

  else if(active_branch==1)
  {
    return (current_cut-max_gain_right);
  }

}


void update_case_1(int whichBlock, int active_branch, int whichNet, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int max_gain_left, int level)
{
  int i;
  int j;
  int k;
  int T;
  if(active_branch==0)
  {
    T=net[whichNet].T_left;
  }

  else if(active_branch==1)
  {
    T=net[whichNet].T_right;
  }

  // For current net check the T 
  if(T==0)
  {
    for(j=0; j<net[whichNet].num_pins; j++)
    {
      int whichPin=net[whichNet].pins[j];
      int markedPins[net[whichNet].num_pins];
      for(i=0; i<net[whichNet].num_pins;i++)
      markedPins[i]=-1;

      if(array_scan(whichPin,markedPins, net[whichNet].num_pins)==0)
      {
      //printf("Duplicate Pin!\n");
      continue;
      }
      markedPins[j]=whichPin;

      #ifdef DEBUG_PART_PLACE
      printf("looking at pin %d block %s , left = %d on net %d\n", j,block[whichPin].name,block[whichPin].left, whichNet);
      printf("is free: %d, is global clock = %d global clock is %d\n",block[whichPin].free, block[net[j].pins[0]].loc, GLOBAL_CLOCK_LOC);
      #endif 

      

      if(block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC && whichPin != whichBlock && block[whichPin].level==level)
      {
        
        #ifdef DEBUG_PART_PLACE
        printf("Case 1\n");
        printf(" T=0, Gain: %d\n",block[whichPin].level_gain[level]);
        #endif

        block[whichPin].level_gain[level]=block[whichPin].level_gain[level]+1;
        
        #ifdef DEBUG_PART_PLACE
        printf(" T=0, Updated Gain: %d on block %s\n",block[whichPin].level_gain[level], block[whichPin].name);
        printf("Before Insert\n");
        //printList(buckets_left[block[block_oper].gain]);
        #endif

// CHECKED UP TO HERE

        #ifdef CHECK_GAINS
        check_gains(whichPin);
        printf("Case 1\n");
        printf("On block %s and net %d and branch %d\n",block[whichPin].name,whichNet, active_branch);                  
        #endif
        
      }
    }
  }
}

void update_case_2(int whichBlock, int active_branch, int whichNet, ListPtr *buckets_left, ListPtr *buckets_right, int pmax,int max_gain_right, int max_gain_left, int level)
{
  int i;
  int j;
  int k;
  int T;
  if(active_branch==0)
  {
    T=net[whichNet].T_left;
  }

  else if(active_branch==1)
  {
    T=net[whichNet].T_right;
  }

  if(T==1)
    {
      for(j=0; j<net[whichNet].num_pins; j++)
      {
        int whichPin=net[whichNet].pins[j];
        int markedPins[net[whichNet].num_pins];
        for(i=0; i<net[whichNet].num_pins;i++)
        markedPins[i]=-1;

        if(array_scan(whichPin,markedPins, net[whichNet].num_pins)==0)
        {
          #ifdef DEBUG_PART_PLACE
          printf("Duplicate Pin!\n");
          #endif
          continue;
        }
        markedPins[j]=whichPin;

        if(((block[whichPin].left==0 && active_branch==0) || (block[whichPin].left==1 && active_branch==1)) && whichPin != whichBlock)
        //if(((block[whichPin].left==0 && active_branch==0) || (block[whichPin].left==1 && active_branch==1))&& block[whichPin].free==1)
        {

          #ifdef DEBUG_PART_PLACE
          printf("Left Case 2\n");
          printf(" T_left =1, Gain: %d\n",block[whichPin].gain=block[whichPin].gain);
          #endif

          block[whichPin].gain=block[whichPin].gain-1;

          #ifdef DEBUG_PART_PLACE
          printf(" Gain-1, T_left =1, Updated Gain: %d, on Block %s, %d\n",block[whichPin].gain,block[whichPin].name,whichPin);
          #endif

          #ifdef CHECK_GAINS
          check_gains(whichPin);
          printf("Left Case 2\n");
          printf("On block %s and net %d\n",block[whichPin].name,whichNet);   
          #endif

          if(active_branch==0)
          {
            insert(&buckets_right[(block[whichPin].gain)+pmax], whichPin);
            delete(&buckets_right[(block[whichPin].gain)+pmax+1], whichPin);
          }
          else if (active_branch==1)
          {
            insert(&buckets_left[(block[whichPin].gain)+pmax], whichPin);
            delete(&buckets_left[(block[whichPin].gain)+pmax+1], whichPin);            
          }

          #ifdef DEBUG_PART_PLACE
          printf("Completed Left Case 2\n");
          #endif

        }
      }
    }
}


void update_case_move(int active_branch, int whichBlock, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int max_gain_left, int start, int numberOfNets)
{
  int i;
  int j;
  int k;
  // Move Block and update F and T for each net.
  #ifdef DEBUG_PART_PLACE
    printf("Begin Left Move Active Block\n");
  #endif

  // Move base cells and 
    if(active_branch==0)
    {
      block[whichBlock].left=0;
      #ifdef DEBUG_PART_PLACE
      printf("Left Brnach block moved: %s, left %d\n", block[whichBlock].name, block[whichBlock].left);
      #endif
      block[whichBlock].free=0;
      block[whichBlock].gain=-max_gain_left;
      insert(&buckets_right[-max_gain_left+pmax], whichBlock);     
    }
    else if (active_branch==1)
    {
      block[whichBlock].left=1;
      #ifdef DEBUG_PART_PLACE
      printf("Right Branch block moved: %s, left %d\n", block[whichBlock].name, block[whichBlock].left);
      #endif
      block[whichBlock].free=0;
      block[whichBlock].gain=-max_gain_right;
      #ifdef DEBUG_PART_PLACE
      printf("max gain right = %d\n", max_gain_right);
      printf("Block %s gain updated to %d\n",block[whichBlock].name, block[whichBlock].gain);
      #endif
      insert(&buckets_left[-max_gain_right+pmax], whichBlock);
    }

    if(active_branch==0)
    {
      numberOfNets=net_max_index(whichBlock);

        int markedNets[MAXLUT+2];
        for(i=0; i<MAXLUT+2;i++)
          markedNets[i]=-1;

      for(i=0; i<numberOfNets; i++)
      {
        int whichNet=block[whichBlock].nets[i];

        if(array_scan(whichNet,markedNets,MAXLUT+2)==0)
        {
          continue;
        }
        markedNets[i]=whichNet;
        net[whichNet].T_left = net[whichNet].T_left+1;
        net[whichNet].F_right=net[whichNet].T_left;
        net[whichNet].F_left = net[whichNet].F_left-1;
        net[whichNet].T_right=net[whichNet].F_left;
      }
    }

  else if (active_branch==1)
  {
    numberOfNets=net_max_index(whichBlock);
    int markedNets[MAXLUT+2];
      for(i=0; i<MAXLUT+2;i++)
        markedNets[i]=-1;
    //printf("number of nets to check %d\n",numberOfNets);
    for(i=0; i<numberOfNets; i++)
    {
      int whichNet=block[whichBlock].nets[i];
      if(array_scan(whichNet,markedNets,MAXLUT+2)==0)
      {
        //printf("shit\n");
        continue;
      }
      markedNets[i]=whichNet;
      net[whichNet].T_right = net[whichNet].T_right+1;
      net[whichNet].F_left=net[whichNet].T_right;
      net[whichNet].F_right = net[whichNet].F_right-1;
      //printf("Net %d F_right %d\n", whichNet, net[whichNet].F_right);
      net[whichNet].T_left=net[whichNet].F_right;
    }
  }

  if(active_branch==0)
  {
    #ifdef DEBUG_PART_PLACE
    printf("Block for move index %d, name : %s, gain: %d\n", whichBlock, block[whichBlock].name, max_gain_left);
    printList(buckets_left[max_gain_left+pmax]); 
    #endif
    delete(&buckets_left[max_gain_left+pmax], whichBlock);
    #ifdef DEBUG_PART_PLACE
    printf("New list with deletion\n");
    printList(buckets_left[max_gain_left+pmax]); 
    printf("End Left Move Active Block\n");
    #endif
  }
  else if(active_branch==1)
  { 
  #ifdef DEBUG_PART_PLACE
    printf("Block for move index %d, name : %s, gain: %d\n", whichBlock, block[whichBlock].name, max_gain_right);
    printList(buckets_right[max_gain_right+pmax]); 
  #endif
    delete(&buckets_right[max_gain_right+pmax], whichBlock);
  #ifdef DEBUG_PART_PLACE
    printf("New list with deletion\n");
    printList(buckets_right[max_gain_right+pmax]); 
    printf("End Right Move Active Block\n");
  #endif
  }
 

}

void update_case_3(int whichBlock, int active_branch, int whichNet, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int max_gain_left, int level)
{
  int i;
  int j;
  int k;
  int F;
  if(active_branch==0)
  {
    F=net[whichNet].F_left;
  }
  else if(active_branch==1)
  {
    F=net[whichNet].F_right;
  }
  //printf("net: %d, F %d\n", whichNet, F);
  if(F==0)
  {
    for(j=0; j<net[whichNet].num_pins; j++)
    {
      int whichPin=net[whichNet].pins[j];
      int markedPins[net[whichNet].num_pins];
        for(i=0; i<net[whichNet].num_pins;i++)
        markedPins[i]=-1;

        if(array_scan(whichPin,markedPins, net[whichNet].num_pins)==0)
        {
          //printf("Duplicate Pin!\n");
          continue;
        }
        markedPins[j]=whichPin;

      if(block[whichPin].loc!=GLOBAL_CLOCK_LOC && whichPin != whichBlock)
      //if(block[whichPin].free==1 && block[whichPin].loc!=GLOBAL_CLOCK_LOC)
      {
        
        #ifdef DEBUG_PART_PLACE
        printf(" F_left =0, Gain: %d\n",block[whichPin].gain);
        #endif

        block[whichPin].gain=block[whichPin].gain-1;

        #ifdef DEBUG_PART_PLACE
        printf(" Gain -1 F_left =0, Updated Gain: %d, on Block %s, %d\n",block[whichPin].gain,block[whichPin].name,whichPin);
        #endif

        #ifdef CHECK_GAINS
        check_gains(whichPin);
        printf("Left Case 3\n");
        printf("On block %s and net %d\n",block[buckets_right[(-max_gain_left)+pmax]->cellNumber].name,block[buckets_right[(-max_gain_left)+pmax]->cellNumber].nets[i]);
        #endif

        if(active_branch==0)
        {
          insert(&buckets_right[(block[whichPin].gain)+pmax ], whichPin);
          delete(&buckets_right[(block[whichPin].gain)+pmax+1], whichPin);
        }

        else if (active_branch ==1)
        {
          insert(&buckets_left[(block[whichPin].gain)+pmax ], whichPin);
          delete(&buckets_left[(block[whichPin].gain)+pmax+1], whichPin);          
        }
      }
    }
  }
}

void update_case_4(int whichBlock, int active_branch, int whichNet, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int max_gain_left, int level)
{
  int i;
  int j;
  int k;
  int F;
  if(active_branch==0)
  {
    F=net[whichNet].F_left;
  }
  else if(active_branch==1)
  {
    F=net[whichNet].F_right;
  }

  if(net[whichNet].F_left==1)
  {
    for(j=0; j<net[whichNet].num_pins; j++)
    {
      int whichPin=net[whichNet].pins[j];
      int markedPins[net[whichNet].num_pins];
        for(i=0; i<net[whichNet].num_pins;i++)
        markedPins[i]=-1;

        if(array_scan(whichPin,markedPins, net[whichNet].num_pins)==0)
        {
          //printf("Duplicate Pin!\n");
          continue;
        }
        //printf("whichPin %s left = %d free = %d active branch= %d\n", block[whichPin].name, block[whichPin].left, block[whichPin].free, active_branch);

      if(((block[whichPin].left==1 && active_branch==0) || (block[whichPin].left==0 && active_branch==1)) && whichPin != whichBlock)
      //if(((block[whichPin].left==1 && active_branch==0) || (block[whichPin].left==0 && active_branch==1))&& block[whichPin].free==1)
      {

        #ifdef DEBUG_PART_PLACE             
        printf(" F_left =1, Gain: %d\n",block[whichPin].gain=block[whichPin].gain);
        #endif

        block[whichPin].gain=block[whichPin].gain+1;
        
        #ifdef DEBUG_PART_PLACE
        printf(" Gain +1 F_left =1, Updated Gain: %d, on Block %s, %d\n",block[whichPin].gain,block[whichPin].name,whichPin);
        #endif

        #ifdef CHECK_GAINS
        check_gains(whichPin);
        printf("Left Case 4\n");
        printf("On block %s and net %d\n",block[buckets_right[(-max_gain_left)+pmax]->cellNumber].name,block[buckets_right[(-max_gain_left)+pmax]->cellNumber].nets[i]);
        #endif
        if (active_branch==0)
        {
          insert(&buckets_left[(block[whichPin].gain)+pmax], whichPin);
          delete(&buckets_left[(block[whichPin].gain)+pmax-1], whichPin);
        }

        else if (active_branch==1)
        {
          insert(&buckets_right[(block[whichPin].gain)+pmax], whichPin);
          delete(&buckets_right[(block[whichPin].gain)+pmax-1], whichPin);          
        }
      }
    }
  }
}

int net_max_index(int whichBlock)
{
  int numberOfNets;
  // Figure out how many nets are connected to the block
  if(block[whichBlock].type==LATCH || block[whichBlock].type==LUT_AND_LATCH)
  {
  numberOfNets=block[whichBlock].num_nets-1;

  }

  else
  {
  numberOfNets=block[whichBlock].num_nets;

  }
  return numberOfNets;
}
