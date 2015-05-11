/* written by andre 1/2011 ...removed mesh stuff 1/2015 */

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "tree.h"
#include "main.h"
#include "globals.h"
#include "asap.h"
#include "queue.h"


/* #define VERBOSE_ASAP_DELAY 1 */
#define VERBOSE_ASAP_SCHEDULE

int *level=(int *)NULL;

int compute_level(int whichblock)
{
  int low, high, ip;

  int max_predecessor_level=0;

  if (block[whichblock].type==INPAD)
    return(0);

  if (block[whichblock].type==OUTPAD)
    low=0;
  else 
    low=1;

  if ((block[whichblock].type==LATCH) || (block[whichblock].type==LUT_AND_LATCH))
    high=block[whichblock].num_nets-1; // leave out clock
  else
    high=block[whichblock].num_nets;


  for(ip=low;ip<high;ip++)
    {
      if ((block[net[block[whichblock].nets[ip]].pins[0]].type==LATCH)
	  || (block[net[block[whichblock].nets[ip]].pins[0]].type==LUT_AND_LATCH))
	{
	  // these are at zero
	}
      else
	if (level[net[block[whichblock].nets[ip]].pins[0]]==LEVEL_UNDETERMINED)
	  {
	    return(LEVEL_UNDETERMINED);
	  }
	else
	  {
	    max_predecessor_level=max(max_predecessor_level,level[net[block[whichblock].nets[ip]].pins[0]]);
	  }
    }
  return(max_predecessor_level+1);
}

int asap_delay()
{
  int i;
  int max_timestep=0;

  // allocate level annotation array
  level = (int *)my_malloc(sizeof(int)*num_blocks);
  int *ready_queue=new_queue(num_blocks);

  // initialize levels
  for (i=0;i<num_blocks;i++)
    if (block[i].type==INPAD)
      level[i]=0;
    else
      level[i]=LEVEL_UNDETERMINED;

  // see what's ready now and fill ready queue
  for (i=0;i<num_blocks;i++)
    {
      if (block[i].type==INPAD)
	{
	  // skip already processed
	}
      else 
	{
	  int blevel=compute_level(i);
	  if (blevel!=LEVEL_UNDETERMINED)
	    {
	      level[i]=blevel;
	      max_timestep=max(max_timestep,blevel);
	      push_queue(ready_queue,i);
	    }
	}
    }

  // process ready queue (assign levels once predecessor levels assigned)
  while(empty_queue(ready_queue)==FALSE)
    {
      int next_block=pop_queue(ready_queue);

      if (level[next_block]==LEVEL_UNDETERMINED)
	{
	  fprintf(stderr,"asap.asap_delay consistency error: found %s (%d) on ready queue, but level not set.\n",
		  block[next_block].name, next_block);
	  exit(13);
	}

#ifdef VERBOSE_ASAP_DELAY
      fprintf(stdout,"Processing %s (%d) at level %d.\n",
	      block[next_block].name, next_block,level[next_block]);
#endif      

      if ((block[next_block].type==OUTPAD)||
	  (block[next_block].type==LATCH) || 
	  (block[next_block].type==LUT_AND_LATCH))
	{
	  // don't need to process successors 
	  //   no successors for output
	  //   already know delay for latch case
	}
      else
	{
	  // check successors, try compute level, assign and push on queue when ready
	  for(i=1;i<net[block[next_block].nets[0]].num_pins;i++)
	    {
	      int successor_block=net[block[next_block].nets[0]].pins[i];
	      int blevel=compute_level(successor_block);
	      if (blevel!=LEVEL_UNDETERMINED)
		{
		  level[successor_block]=blevel;
		  max_timestep=max(max_timestep,blevel);
		  push_queue(ready_queue,successor_block);
		}
	    }
	}

    }


  free_queue(ready_queue);
  return(max_timestep);    

}

void print_level_assignment()
{
  int i;
  for (i=0;i<num_blocks;i++)
    fprintf(stdout,"%s %d\n",block[i].name,level[i]);
    
}

