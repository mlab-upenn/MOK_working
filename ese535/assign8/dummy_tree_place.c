#include <stdio.h>
#include <string.h>
#include "util.h"
#include "main.h"
#include "globals.h"
#include "tree.h"
#include "asap.h"
#include "domain.h"

void random_place(int size, int cluster_size, boolean *is_clock, boolean global_clock)
{

  int i,j;
  int pes=get_size();
  int available=pes*cluster_size;

  my_srandom(3723); /* seed for repeatable results for assign2 */

  /* places will be used to keep track of the remaining cluster slots */
  int *places=(int *)malloc(available*sizeof(int));
  for (i=0;i<pes;i++)
    for (j=0;j<cluster_size;j++)
      places[i*cluster_size+j]=i;

  for (i=0;i<num_blocks;i++)
    {

      if ((global_clock==TRUE) && (block[i].type==INPAD) && (is_clock[block[i].nets[0]]==TRUE))
	{
	  // skip placing clock, but mark it so doesn't complain later
	  block[i].loc=GLOBAL_CLOCK_LOC;
	  block[i].slot_loc=GLOBAL_CLOCK_LOC;
	}
      else
	{

	  /* place each block in one of the remaining cluster slots */
	  int place=my_irand(available-1);
	  /* printf("with available=%d, placing %d (%s) in loc=%d\n",available,i,block[i].name,places[place]);  */
	  place_block(i, /* this block */
		      places[place], /* in the random selection */
		      level[i] /* at its ASAP time */
		      ); 
	  places[place]=places[available-1];
	  available--;
	}
    }

  free(places);
  return;

}

void dummy_tree_place(int size, int cluster_size, boolean *is_clock, boolean global_clock)
{

  int i;
  int *growth_schedule;
  int growth_schedule_length=ilog2(size);
  growth_schedule=(int *)malloc(growth_schedule_length*sizeof(int));
  for (i=0;i<growth_schedule_length;i++)
    growth_schedule[i]=1;

  allocate_tree(growth_schedule,
		growth_schedule_length,
		INITIAL_DOMAINS,
		cluster_size,// blocks per PE
		cluster_size// io limits, not currently used
		);

  random_place(size,cluster_size,is_clock,global_clock);

  return;

}

/* mostly for debug */
void dummy_tree_place_schedule(int *growth_schedule, int growth_schedule_length, int cluster_size, boolean *is_clock, boolean global_clock)
{


  allocate_tree(growth_schedule,
		growth_schedule_length,
		INITIAL_DOMAINS,
		cluster_size,// blocks per PE
		cluster_size// io limits, not currently used
		);

  random_place(1<<growth_schedule_length,cluster_size,is_clock,global_clock);

  return;

}
