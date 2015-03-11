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
int calculate_cut(boolean *left)
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

// Compute pmax
  pmax=compute_pmax();

#ifdef DEBUG_PART_PLACE
  printf("pmax equals %d\n", pmax);
#endif

// Initialize max gains
  max_gain_left=-1*pmax;
  max_gain_right=-1*pmax;

// Intialize buckets
  ListPtr *buckets_left;
  buckets_left=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

  ListPtr *buckets_right;
  buckets_right=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

  ListPtr *free_buckets_left;
  free_buckets_left=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

  ListPtr *free_buckets_right;
  free_buckets_right=(ListPtr*)malloc(((pmax*2)+1)*sizeof(ListPtr));

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
// If block is not a clock place block i at the root at timestep i.
    else
      insert_block_switch(i,root,level[i]);
  }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Initialization */

  initial_partition(root, cluster_size);
  //initialize_F_and_T(buckets_left,buckets_right,left);
  max_gain_left=initialize_left_branch(is_clock, left, right, root, gain, count, max_gain_left, blockInfo, pmax, buckets_left);
  #ifdef DEBUG_PART_PLACE
  printf("Initial max gain left: %d\n",max_gain_left);
  #endif
  max_gain_right=initialize_right_branch(is_clock, left, right, root, gain, count, max_gain_right, blockInfo, pmax, buckets_right);
  #ifdef DEBUG_PART_PLACE
  printf("Initial max gain right: %d\n",max_gain_right);
  #endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Initialization Complete */
// Determine which branch

  int current_cut=calculate_cut(left);
  int previous_cut=current_cut+1;
  int passes=0;
while(current_cut<previous_cut && passes<10)
{
  passes=passes+1;

  //#ifdef DEBUG_PART_PLACE 
  printf("Pass Number: %d\n",passes);
  //#endif

  free_blocks();

  while(locked_nodes<(num_blocks-1)&& ((max_gain_right>=0 && max_gain_right>= max_gain_left) || ( max_gain_left>=0 && max_gain_left>= max_gain_right)))
  {
// Find Max Gain Left
    max_gain_left=compute_max_gain_left(pmax, buckets_left);
    #ifdef DEBUG_PART_PLACE
    printf("max gain left: %d\n",max_gain_left);
    #endif

// Find Max Gain Right
    max_gain_right=compute_max_gain_right(pmax, buckets_right);
    #ifdef DEBUG_PART_PLACE
    printf("max gain right: %d\n",max_gain_right);
    #endif
// Select active branch
    active_branch=which_branch(max_gain_left,max_gain_right,left, pmax, active_branch, buckets_left, buckets_right);
    if(active_branch==-1)
      break;
    previous_cut=current_cut;
// If active branch is left
    current_cut=update_gains_left(active_branch, buckets_left, buckets_right, pmax, max_gain_left, left, current_cut);
// If active branch is right
    current_cut=update_gains_right(active_branch, buckets_left, buckets_right, pmax, max_gain_right, left, current_cut);
    #ifdef DEBUG_PART_PLACE
    printf("Current Cut after update %d\n", current_cut);
    #endif

    locked_nodes=locked_nodes+1;
  }


#ifdef DEBUG_PART_PLACE
  printf("Old cut %d\n", previous_cut);
  printf("Current cut %d\n", current_cut);
#endif
}
#ifdef DEBUG_PART_PLACE 
  printf("Current cut %d\n", current_cut);
#endif

return(current_cut);
}
