#include <stdio.h>
#include <string.h>
#include "util.h"
#include "main.h"
#include "globals.h"
#include "tree.h"
#include "asap.h"
#include "domain.h"
#include "part_place.h"
#include "list.h"
#include "klfm.h"
//#define DEBUG_PART_PLACE 


int root_switch(int height)
{
  int i;
  int first=get_size();
  int nodes=get_nodes();
  for(i=first;i<nodes;i++)
    if (tree_location[i].height==height)
      return(i);

    return(-1);
  }

  boolean mixed_net(int whichnet, boolean *left)
  {
    int i;
    boolean source_side=left[net[whichnet].pins[0]];
    for (i=1;i<net[whichnet].num_pins;i++)
    {
      if (left[net[whichnet].pins[i]]!=source_side)
      {
        return(TRUE);
      }
    }
    return(FALSE);
  }

  

/* for reporting out cut on the non-recursive partition case,
   where we cannot use the assign2 cost function;
   this is designed *only* to work for top cut,
   so don't plan on reusing it for cuts lower in tree. */
   void calculate_and_write_top_cut(int size, char *cost_file, boolean verbose, int current_cut)
   {

    FILE *fp; 
    int cut=0;
    boolean *left=(boolean *)malloc(sizeof(int)*num_blocks);
    int growth_schedule_length=ilog2(size);
    int i;


    int root=root_switch(growth_schedule_length);
    if (root<0)
    {
      fprintf(stderr,"Could not find root switch\n");
      exit(21);
    }

    for (i=0;i<num_blocks;i++)
      left[i]=FALSE;

    if (verbose==TRUE)
      printf("LEFT: ");
    for (i=0;i<tree_location[tree_location[root].left].used_slots;i++)
    {
      left[tree_location[tree_location[root].left].slots[i]]=TRUE;
      if (verbose==TRUE)
       printf(" %s",block[tree_location[tree_location[root].left].slots[i]].name);
   }
   if (verbose==TRUE)
   {
    printf("\nRIGHT: ");
    for (i=0;i<tree_location[tree_location[root].right].used_slots;i++)
    {
     printf(" %s",block[tree_location[tree_location[root].right].slots[i]].name);
   }
   printf("\nSHARED NETS: ");
 }

 for (i=0;i<num_nets;i++)
    if (block[net[i].pins[0]].loc!=GLOBAL_CLOCK_LOC) // skip clock
      if (mixed_net(i,left)==TRUE)
      {
        cut++;
      }
      if (verbose==TRUE)
        printf("\n");

  // write out result
      fp = my_fopen(cost_file,"w",0);
      fprintf(fp,"Number of blocks:%d,Initial Cut%d, KLFM cut: %d\n",num_blocks,cut,current_cut);
      free(left);
      fclose(fp);

    }


    int calculate_cut(boolean *left)
    {

      int cut=0;
      int i;
      for (i=0;i<num_nets;i++)
      if (block[net[i].pins[0]].loc!=GLOBAL_CLOCK_LOC) // skip clock
      if (mixed_net(i,left)==TRUE)
      {
        cut++;
      }
      return (cut);
    }

   int part_place(int size, int cluster_size, boolean recurse, boolean level_max, boolean verbose, boolean *is_clock, boolean global_clock)
    {
    /* Local variables */
      int i;
      int j;
      int k;
      int *growth_schedule;
      int growth_schedule_length=ilog2(size);
      int *gain;
      static int count;
      gain= (int *)malloc(num_blocks*sizeof(int));
      boolean *left=(boolean *)malloc(sizeof(int)*num_blocks);
      boolean *right=(boolean *)malloc(sizeof(int)*num_blocks);
      static int max_gain_left=0;
      static int max_gain_right=0;
      int pmax=0;
      int active_branch=0;
      int locked_nodes=0;


  // Start with P=0 schedule for global route
  // Allocate memory for growth schedule
      growth_schedule=(int *)malloc(growth_schedule_length*sizeof(int));

  // Fill in growth schedule with 1.
      for (i=0;i<growth_schedule_length;i++)
        growth_schedule[i]=1;
  // Initialize Gain array
      for (i=0; i<num_blocks; i++)
        gain[i]=0;

  // Create block info structure
      struct block_info *blockInfo;
      blockInfo =(struct block_info*)malloc(num_blocks*sizeof(struct block_info));
      pmax=compute_pmax();

  // Determine the value of pmax
      /*for (i=0; i<num_nets;i++)
      {
        if (net[i].num_pins>pmax)
        {
          pmax=net[i].num_pins;
        }
      }*/
      #ifdef DEBUG_PART_PLACE
      printf("pmax equals %d\n", pmax);
      #endif
      max_gain_left=-1*pmax;
      max_gain_right=-1*pmax;

      ListPtr *buckets_left;
      buckets_left=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

      ListPtr *buckets_right;
      buckets_right=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

      ListPtr *free_buckets_left;
      free_buckets_left=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

      ListPtr *free_buckets_right;
      free_buckets_right=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

  /* Create the tree structure */
      allocate_tree(growth_schedule,
        growth_schedule_length,
        INITIAL_DOMAINS,
      cluster_size,// blocks per PE
      cluster_size// io limits, not currently used
      );

  // Find the root
      int root=root_switch(growth_schedule_length);
      if (root<0)
      {
        fprintf(stderr,"Could not find root switch\n");
        exit(20);
      }

  /* Place blocks */
      for (i=0;i<num_blocks;i++)
      {
        if ((global_clock==TRUE) && (block[i].type==INPAD) && (is_clock[block[i].nets[0]]==TRUE))
        {
	  // Skip placing clock, but mark it so doesn't complain later
         block[i].loc=GLOBAL_CLOCK_LOC;
         block[i].slot_loc=GLOBAL_CLOCK_LOC;
       }
    // If block is not a clock place block i at the root at timestep i.
       else
         insert_block_switch(i,root,level[i]);
     }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* INITIAL PARTITION */

  /* Naively split blocks into two halves */
     int part_size=(get_size()*cluster_size)>>1;
   //int part_size=(get_size())/2;
   //printf("Partition Size: %d, get_size %d, cluster size %d", part_size,get_size(), cluster_size);
     for (i=0;i<tree_location[root].used_slots;i++)
    // First half
      if (i<part_size)
        insert_block_switch(tree_location[root].slots[i],tree_location[root].left,level[tree_location[root].slots[i]]);
    // Second half
      else
        insert_block_switch(tree_location[root].slots[i],tree_location[root].right,level[tree_location[root].slots[i]]);

  /* TO DO: Unlock all blocks */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* LEFT BRANCH INITIALIZATION */
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
        // If the net has a pin on both sides of the partition
          if(mixed_net(block[current_block].nets[j],left) == TRUE)
          {

        // Initialize count to zero
            count=0;
        // For each pins on the net
            for (k=1; k<net[j].num_pins; k++)
            {
          // If the block associated with the pin is on the left hand side
              if (left[net[j].pins[k]]==TRUE)
              {
            // Increase the count
                count=count+1;
                net[block[current_block].nets[j]].F_left=count;
              }
            }
        // If there is not more than one pin with a block on the left hand side
            if (count<=1)
            {
        // Increment the gain
              gain[i]=gain[i]+1;
            }
          }

      // If this is not a mixed net decrement the gain (moving this block relative to this net will make things worse)
          else 
          {
            gain[i]=gain[i]-1;
            net[block[current_block].nets[j]].T_left=numberOfNets-start;
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
        buckets_left[i]=(ListNode*)malloc(sizeof(ListNode));
        buckets_left[i]->nextPtr=NULL;
        buckets_left[i]->cellNumber=NULL;
        for (j=0;j<tree_location[tree_location[root].left].used_slots;j++)
        {
 if(blockInfo[j].gain==(i-(pmax)))
          {
            
                insert(&buckets_left[i],blockInfo[j].block_address);
              
            }
        } 
        #ifdef DEBUG_PART_PLACE
        printf("Gain: %d\n", (i-(pmax)));
        printList(buckets_left[i]); 
        #endif
      }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* RIGHT BRANCH */
  // Populate array "left" with boolean values regarding whether a block is in the left or right branch of the tree
  // Initialize left with all FALSE.  
      for (i=0;i<num_blocks;i++)
      {
        right[i]=FALSE;
      }
  // For all the blocks placed on the left side change left array value to TRUE
      for (i=0;i<tree_location[tree_location[root].right].used_slots;i++)
      {
        right[tree_location[tree_location[root].left].slots[i]]=TRUE;
      }

  // For each block on the left branch
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

    // For all the nets
        for (j=start; j<numberOfNets; j++)
        {
        // If the net has a pin on both sides of the partition
          if(mixed_net(block[current_block].nets[j],right) == TRUE)
          {
        // Initialize count to zero
            count=0;
        // For each pins on the net
            for (k=1; k<net[j].num_pins; k++)
            {
          // If the block associated with the pin is on the left hand side
              if (left[net[j].pins[k]]==TRUE)
              {
            // Increase the count
                count=count+1;
                net[block[current_block].nets[j]].F_right=count;
              }
            }
        // If there is not more than one pin with a block on the left hand side
            if (count<=1)
            {
        // Increment the gain
              gain[i]=gain[i]+1;
            }
          }
      // If this is not a mixed net decrement the gain (moving this block relative to this net will make things worse)
          else 
          {
            gain[i]=gain[i]-1;
            net[block[current_block].nets[j]].T_right=numberOfNets-start;
          }
        }
    // Keep track of the maximum gain for indexing the buckets array later...
        if (gain[i]>= max_gain_right)
        {
          max_gain_right=gain[i];
        }
        #ifdef DEBUG_PART_PLACE
        printf("Gain %d on block %s\n", gain[i], block[current_block].name);
        #endif

        blockInfo[current_block].block_address=current_block;
        blockInfo[current_block].name=block[current_block].name;
        blockInfo[current_block].gain=gain[i];
        blockInfo[current_block].left=right[i];
        blockInfo[current_block].free=TRUE;

        block[current_block].gain=gain[i];
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
        buckets_right[i]->cellNumber=NULL;

        for (j=(num_blocks-tree_location[tree_location[root].right].used_slots);j<num_blocks;j++)
        {
          if(blockInfo[j].gain==(i-(pmax)))
          {
          
              
                insert(&buckets_right[i],blockInfo[j].block_address);
              }
            
          } 
        #ifdef DEBUG_PART_PLACE
          printf("Gain: %d\n", (i-(pmax)));
          printList(buckets_right[i]); 
        #endif
        }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Initialization Complete */
// Determine which branch

       int current_cut=calculate_cut(left);
       int previous_cut=current_cut+1;
       int passes=0;
      //while(current_cut<previous_cut && passes<10)
      //{
        passes=passes+1;
        #ifdef DEBUG_PART_PLACE 
        printf("Pass Number: %d\n",passes);
        #endif
        for (i=0; i<num_blocks; i++)
        {
          block[i].free=1;
        }
        
        while(locked_nodes<(num_blocks-1) && ((max_gain_right>=0 && max_gain_right> max_gain_left) || ( max_gain_left>=0 && max_gain_left> max_gain_right)))
        {
          int left_count=0;
          int right_count=0;
        // Reinitialize max gains
          max_gain_left=-pmax;
          max_gain_right=-pmax;
        // Check gain on all blocks
          for (i=0; i<num_blocks;i++)
          {
            #ifdef DEBUG_PART_PLACE
            printf("Block: %s, i= %d, is free: %d, gain is: %d, is left? %d\n", block[i].name,i, block[i].free, block[i].gain, block[i].left);
            #endif

            if(block[i].loc != GLOBAL_CLOCK_LOC && block[i].type!=-1)
            {
              if (block[i].left==1)
              {
                left[i]=TRUE;
                if(block[i].free==1)
                {
                  left_count=left_count+1;
                }
                if(block[i].gain>max_gain_left)
                {
                  max_gain_left=block[i].gain;
                }
              }
              else if(block[i].left==0) 
              {
                left[i]=FALSE;
                if (block[i].free==1)
                {
                  right_count=right_count+1;
                }
                if(block[i].gain>max_gain_right)
                {
                  max_gain_right=block[i].gain;
                }
              }
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
            #ifdef DEBUG_PART_PLACE
            printf("By balance Right Branch Active\n");
            #endif
          }
          else if(balance>=r)
          {
            active_branch=0;
            #ifdef DEBUG_PART_PLACE
            printf("By balance Left Branch Active\n");
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Update gains */
// If the active block is in the left partition
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
            }
      // Before move check the nets in the "to block" ie any nets with a pin in the right half
            for(i=start; i<numberOfNets; i++)
            {
        // Using index i, pick up the current net:
              int current_net=block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i];
        // For current net check the T 
              if(net[current_net].T_left==0)
              {
                for(j=0; j<net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].num_pins; j++)
                {
                  //printf("looking at pin %d\n", j);
                  int block_oper=net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].pins[j];


                  if(block[block_oper].left==1 && block[block_oper].free==1 &&  block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
                  {
                    printf("Left Case 1\n");
                    #ifdef DEBUG_PART_PLACE
                    printf(" T_left =0, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                    #endif
                    block[block_oper].gain=block[block_oper].gain+1;
                    #ifdef DEBUG_PART_PLACE
                    printf(" T_left =0, Updated Gain: %d\n",block[block_oper].gain);
                    printf("Before Insert\n");
                    #endif
                    insert(&buckets_left[(block[block_oper].gain+pmax)], block_oper);
                    delete(&buckets_left[block[block_oper].gain+pmax-1], block_oper);
                    #ifdef DEBUG_PART_PLACE 
                    printf("After Delete\n");
                    #endif
                    printf("Completed Left Case 1\n");
                  }
                }
              }
              else if(net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].T_left==1)
              {
                for(j=0; j<net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].num_pins; j++)
                {
                  int block_oper=net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].pins[j];

                  if(block[block_oper].left==0 && block[block_oper].free==1)
                  {
                    printf("Left Case 2\n");
                    #ifdef DEBUG_PART_PLACE
                    printf(" T_left =1, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                    #endif
                    block[block_oper].gain=block[block_oper].gain-1;
                    #ifdef DEBUG_PART_PLACE
                    printf(" Gain-1, T_left =1, Updated Gain: %d, on Block %s, %d\n",block[block_oper].gain,block[block_oper].name,block_oper);
                    #endif
                    insert(&buckets_right[(block[block_oper].gain)+pmax], block_oper);
                    delete(&buckets_right[(block[block_oper].gain)+pmax+1], block_oper);
                    printf("Completed Left Case 2\n");

                  }
                }
              }
            }

    // Move Block and update F and T for each net.
            printf("Begin Left Move Active Block\n");
            int block_for_move=buckets_left[(max_gain_left)+pmax]->cellNumber;
            printf("Found block for move\n");
            block[block_for_move].left=0;
            printf("Block for move is now in right branch\n");
            block[block_for_move].free=0;
            printf("Block for move is locked\n");
            insert(&buckets_right[-max_gain_left+pmax], block_for_move);
            printf("Block for move is placed in buckets right\n");
            for(i=start; i<numberOfNets; i++)
            {
             net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].T_left = net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].T_left+1;
             net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].F_left = net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].F_left-1;
           }
           //#ifdef DEBUG_PART_PLACE
           printf("Block for move index %d, name : %s, gain: %d\n", block_for_move, block[block_for_move].name, block[block_for_move].gain);
           //#endif
           printList(buckets_left[max_gain_left+pmax]); 
           printf("We were able to print\n");
           delete(&buckets_left[max_gain_left+pmax], block_for_move);
           printf("End Left Move Active Block\n");

    // Update critical nets after the move
           for(i=start; i<numberOfNets; i++)
           {
            if(net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].F_left==0)
            {
              for(j=0; j<net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].num_pins; j++)
              {
                int block_oper=net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].pins[j];

                if(block[block_oper].left==0 && block[block_oper].free==1 && block[net[j].pins[0]].loc!=GLOBAL_CLOCK_LOC)
                {
                  printf("Left Case 3\n");
                  #ifdef DEBUG_PART_PLACE
                  printf(" F_left =0, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                  #endif
                  block[block_oper].gain=block[block_oper].gain-1;
                  #ifdef DEBUG_PART_PLACE
                  printf(" Gain -1 F_left =0, Updated Gain: %d, on Block %s, %d\n",block[block_oper].gain,block[block_oper].name,block_oper);
                  #endif
                  insert(&buckets_right[(block[block_oper].gain)+pmax ], block_oper);
                  delete(&buckets_right[(block[block_oper].gain)+pmax+1], block_oper);
                  printf("Completed Left Case 3\n");
                }
              }
            }
            else if(net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].F_left==1)
            {
              for(j=0; j<net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].num_pins; j++)
              {
                int block_oper=net[block[buckets_left[(max_gain_left)+pmax]->cellNumber].nets[i]].pins[j];

                if(block[block_oper].left==1 && block[block_oper].free==1)
                {
                  printf("Left Case 4\n");
                  #ifdef DEBUG_PART_PLACE
                  printf(" F_left =1, Gain: %d\n",block[block_oper].gain=block[block_oper].gain);
                  #endif
                  block[block_oper].gain=block[block_oper].gain+1;
                  #ifdef DEBUG_PART_PLACE
                  printf(" F_left =1, Updated Gain: %d\n",block[block_oper].gain);
                  #endif
                  insert(&buckets_left[(block[block_oper].gain)+pmax], block_oper);
                  delete(&buckets_left[(block[block_oper].gain)+pmax-1], block_oper);
                  printf("Completed Left Case 4\n");
                }
              }
            }
          }

        }


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Update Gains */
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
                  printf("Right Case 1\n");
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
                  printf("Right Case 2\n");
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
            printf("Begin Left Move Active Block\n");
            int block_for_move=buckets_right[(max_gain_right)+pmax]->cellNumber;
            printf("Found block for move\n");
            block[block_for_move].left=1;
            printf("Block for move is now in right branch\n");
            block[block_for_move].free=0;
            printf("Block for move is locked\n");
            insert(&buckets_left[-max_gain_right+pmax], block_for_move);
            printf("Block for move is placed in buckets right\n");
            for(i=start; i<numberOfNets; i++)
            {
             net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].T_right = net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].T_right+1;
             net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].F_right = net[block[buckets_right[(max_gain_right)+pmax]->cellNumber].nets[i]].F_right-1;
           }
           //#ifdef DEBUG_PART_PLACE
           printf("Block for move index %d, name : %s, gain: %d\n", block_for_move, block[block_for_move].name, block[block_for_move].gain);
           //#endif
           printList(buckets_right[max_gain_right+pmax]); 
           printf("We were able to print\n");
           delete(&buckets_right[max_gain_right+pmax], block_for_move);
           printf("End Left Move Active Block\n");

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
                printf("Right case 3\n");
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
                printf("Right case 4\n");
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
      }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      locked_nodes=locked_nodes+1;
    }
   
    previous_cut=current_cut;
    printf("This is where it all ended\n");
    current_cut=calculate_cut(left);

    #ifdef DEBUG_PART_PLACE
    printf("Old cut %d\n", previous_cut);
    printf("Current cut %d\n", current_cut);
    #endif
  //}
    #ifdef DEBUG_PART_PLACE 
    printf("Current cut %d\n", current_cut);
    #endif
    return(current_cut);
  }
