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

// Find the root switch
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

// Determine if a net is mixed...
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

// Helper function to calculate the cut before placed in the tree
int calculate_cut(int *left)
{
  int cut=0;
  int i;
  for (i=0;i<num_nets;i++)
  {
  if (block[net[i].pins[0]].loc!=GLOBAL_CLOCK_LOC) // skip clock
    if (mixed_net(i,left)==TRUE)
    {
      cut++;
    }
  }
  return (cut);
}

// Places parts into PEs (eventually) right now performs one bipartition
int part_place(int size, int cluster_size, boolean recurse, boolean level_max, boolean verbose, boolean *is_clock, boolean global_clock)
{
  // Local variables
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
  int current_cut;

  // Allocate memory for growth schedule
  growth_schedule=(int *)malloc(growth_schedule_length*sizeof(int));

  // Fill in growth schedule with 1.
  for (i=0;i<growth_schedule_length;i++)
    growth_schedule[i]=1;


  // Compute pmax
  pmax=compute_pmax();

  #ifdef DEBUG_PART_PLACE
  printf("pmax equals %d\n", pmax);
  #endif

  // Initialize max gains
  max_gain_left=-1*pmax;
  max_gain_right=-1*pmax;

  // Allocate tree
  allocate_tree(growth_schedule, growth_schedule_length, INITIAL_DOMAINS, cluster_size,cluster_size);

  // Find the root
  int root=root_switch(growth_schedule_length);
  if (root<0)
  {
    fprintf(stderr,"Could not find root switch\n");
    exit(20);
  }

  // Place blocks
  for (i=0;i<num_blocks;i++)
  {
    if ((global_clock==TRUE) && (block[i].type==INPAD) && (is_clock[block[i].nets[0]]==TRUE))
    {
      // Skip placing clock, but mark it so doesn't complain later
      block[i].loc=GLOBAL_CLOCK_LOC;
      block[i].slot_loc=GLOBAL_CLOCK_LOC;
    }
    // If block is not a clock place block i at the root at timestep of i.
    else
      insert_block_switch(i,root,level[i]);

  }
//int capacity= (get_size()*cluster_size)>>1;
current_cut=KLFM(tree_location[root].height, pmax, is_clock, root);

printf("Recurse on the tree\n");

//Recursive version for Assign 5

//recurse_tree(tree_location[root].height, root, pmax, is_clock, capacity);

//#ifdef DEBUG_PART_PLACE
for(i=0; i<get_size(); i++)
{
  for(j=0; j<tree_location[i].used_slots;j++)
  {
    printf("PE %d %s\n", i, block[tree_location[i].slots[j]].name);
  }
}
//#endif

return(current_cut);
}
