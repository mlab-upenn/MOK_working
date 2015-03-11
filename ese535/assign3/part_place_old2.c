#include <stdio.h>
#include <string.h>
#include "util.h"
#include "main.h"
#include "globals.h"
#include "tree.h"
#include "asap.h"
#include "domain.h"
#include "part_place.h"

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
   void calculate_and_write_top_cut(int size, char *cost_file, boolean verbose)
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
        printf(" %s",net[i].name);
        cut++;

     }
     if (verbose==TRUE)
      printf("\n");

  // write out result
    fp = my_fopen(cost_file,"w",0);
    fprintf(fp,"%d,%d\n",num_blocks,cut);
    free(left);
    fclose(fp);

  }

  void part_place(int size, int cluster_size, boolean recurse, boolean level_max, boolean verbose, boolean *is_clock, boolean global_clock)
  {
    /* Local variables */
    int i;
    int *growth_schedule;
    int growth_schedule_length=ilog2(size);
    int *gain;
    static int count;
    gain= (int *)malloc(num_blocks*sizeof(int));
    boolean *left=(boolean *)malloc(sizeof(int)*num_blocks);
    boolean *right=(boolean *)malloc(sizeof(int)*num_blocks);
    static int max_gain_left=0;
    static int max_gain_right=0;

  // Start with P=0 schedule for global route
  // Allocate memory for growth schedule
    growth_schedule=(int *)malloc(growth_schedule_length*sizeof(int));
  
  // Fill in growth schedule with 1.
    for (i=0;i<growth_schedule_length;i++)
      growth_schedule[i]=1;
  // Initialize Gain array
    for (int i=0; i<num_blocks; i++)
      gain[i]=0;

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

  // For each block on the netlist
    for (int i=0;i<tree_location[tree_location[root].left].used_slots;i++)
    {
      int numberOfNets=0;
      int start = 0;

    // Figure out how many nets are connected to the block
      if(block[tree_location[tree_location[root].left].slots[i]].type==1 || block[tree_location[tree_location[root].left].slots[i]].type==3)
      {
        numberOfNets=block[tree_location[tree_location[root].left].slots[i]].num_nets-2;
        start=1;
      }
      else
      {
        numberOfNets=block[tree_location[tree_location[root].left].slots[i]].num_nets;
      }
      printf("%d nets on block %s\n", numberOfNets, block[i].name);

    // For all the nets except for the 0th
      for (int j=start; j<numberOfNets; j++)
      {
        // If the net has a pin on both sides of the partition
        if(mixed_net(block[tree_location[tree_location[root].left].slots[i]].nets[j],left) == TRUE)
        {
        //printf("Got a mixed net\n");
        // Initialize count to zero
          count=0;
        // For each pins on the net
          for (int k=1; k<net[j].num_pins; k++)
          {
          // If the block associated with the pin is on the left hand side
            if (left[net[j].pins[k]]==TRUE)
            {
            // Increase the count
              count=count+1;
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
        }
      }
    // Keep track of the maximum gain for indexing the buckets array later...
      if (gain[i]>= max_gain_left)
      {
        max_gain_left=gain[i];
      }

      printf("Gain %d on block %s\n", gain[i], block[i].name);
    }

  // While there remain unlocked blocks and positive gains shuffle

    return;

  }
