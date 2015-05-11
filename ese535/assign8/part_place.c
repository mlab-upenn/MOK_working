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
    if (left[net[whichnet].pins[i]]!=source_side)
      return(TRUE);
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
	  if (verbose==TRUE)
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

  int i;
  int *growth_schedule;
  int growth_schedule_length=ilog2(size);
  // start with P=0 schedule for global route
  growth_schedule=(int *)malloc(growth_schedule_length*sizeof(int));
  for (i=0;i<growth_schedule_length;i++)
    growth_schedule[i]=1;

  // create the tree structure
  allocate_tree(growth_schedule,
		growth_schedule_length,
		INITIAL_DOMAINS,
		cluster_size,// blocks per PE
		cluster_size// io limits, not currently used
		);

  int root=root_switch(growth_schedule_length);
  if (root<0)
    {
      fprintf(stderr,"Could not find root switch\n");
      exit(20);
    }

  for (i=0;i<num_blocks;i++)
    {
      if ((global_clock==TRUE) && (block[i].type==INPAD) && (is_clock[block[i].nets[0]]==TRUE))
	{
	  // skip placing clock, but mark it so doesn't complain later
	  block[i].loc=GLOBAL_CLOCK_LOC;
	  block[i].slot_loc=GLOBAL_CLOCK_LOC;
	}
      else
	insert_block_switch(i,root,level[i]);
    }

  // START YOUR CODE HERE 
  // REPLACE this dummy code with your partition code
  /* begin placeholder code to be replaced */
  /* (1) dummy version simply fills left then right */
  /* (2) note that the way we communicate the partition.
     We store the set of blocks in a subtree in the slots within the switch at the root of the subtree.
     To report the cut size for the single cut (assignment 4, case 3), it will 
     look at the slots in the switch to the left and right of the root.
     This is probably useful for you for recursive partitioning.
     It sets up the next subtree by giving it the set of nodes that are in the half and will, in turn, 
     need to be partitioned. */
  int part_size=(get_size()*cluster_size)>>1; // half of capacity on left, half on right
  for (i=0;i<tree_location[root].used_slots;i++)
    if (i<part_size)
      insert_block_switch(tree_location[root].slots[i],tree_location[root].left,level[tree_location[root].slots[i]]);
    else
      insert_block_switch(tree_location[root].slots[i],tree_location[root].right,level[tree_location[root].slots[i]]);
  /* end placeholder code to be replaced */

  return;

}
