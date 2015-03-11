/* written by andre 1/11 */

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "mesh.h"
#include "sched_main.h"
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

boolean asap_schedule_on_array(int cluster_size, int input_cluster_size)
{

  int current_level;
  int current_position;
  int *sort_index;
  float *sort_key;
  int i;
  int mesh_width;
  int mesh_height;

  mesh_width=get_mesh_width();
  mesh_height=get_mesh_height();


  sort_index=(int *)my_malloc(num_blocks*sizeof(int));
  // just because the sort we have uses heap keys
  sort_key=(float *)my_malloc(num_blocks*sizeof(float));

  for (i=0;i<num_blocks;i++)
    {
      sort_key[i]=(float)level[i];
    }

  // sort by level -- we'll process them by level
  my_heapsort(sort_index,sort_key,num_blocks);


  current_position=0;

  while (current_position<num_blocks)
    {
      int current_level=level[sort_index[current_position]];
      // process all at this level
       
      // restart set of possible current_positions
      int iox=0;
      int ioy=0;
      int lutx=0;
      int luty=0;
      while ((current_position<num_blocks) && 
	     (level[sort_index[current_position]]==current_level))
	{

	  boolean placed=FALSE;
	  int whichblock=sort_index[current_position];
	  if ((block[whichblock].type==INPAD)||(block[whichblock].type==OUTPAD))
	    {
	      // keep looking for a free one
	      while((placed==FALSE)&&(iox<mesh_width))
		{
		    if ((mesh_position_type(iox,ioy)==IOSLOT) && // right type
			(timestep_in_use(iox,ioy,current_level)==FALSE) && // this level available
			(mesh_used_slots(iox,ioy)<input_cluster_size) // space available
			)
		      
		      {

			// DEBUG
			//fprintf(stdout,"Placing %s (%d,%d) level=%d position %d\n",
			//	block[whichblock].name,iox,ioy,current_level,current_position);

			place_block(whichblock,iox,ioy,current_level);
			placed=TRUE;
		      }
		    else
		      {
			ioy++;
			if (ioy==mesh_height)
			  {
			    iox++; 
			    ioy=0;
			  }

		      }

		}

	      if ((iox==mesh_width) && (current_position<num_blocks))
		{

		  free(sort_index);
		  free(sort_key);
		  return(TRUE); // ran out of places to put things before ran out of things to place
		}
	    }
	  else // LUT and FF ccase
	    {
	      // keep looking for a free one
	      while((placed==FALSE)&&(lutx<mesh_width))
		{ 
		  if ((mesh_position_type(lutx,luty)==LUTFF_SLOT) && // right type
		      (timestep_in_use(lutx,luty,current_level)==FALSE) && // this level available
		      (mesh_used_slots(lutx,luty)<cluster_size) // space available
		      )
		      
		    {
		      // DEBUG
		      //fprintf(stdout,"Placing %s (%d,%d) level=%d position %d\n",
		      //     block[whichblock].name,lutx,luty,current_level,current_position);
		      
		      place_block(whichblock,lutx,luty,current_level);
		      placed=TRUE;
		    }
		  else
		    {
		      luty++;
		      if (luty==mesh_height)
			{
			  lutx++;
			  luty=0;
			}


		    }
		}

	      if ((lutx==mesh_width) && (current_position<num_blocks))
		{

		  free(sort_index);
		  free(sort_key);
		  return(TRUE); // ran out of places to put things before ran out of things to place
		}

	    }


	  current_position++;
	}
    }
  

  free(sort_index);
  free(sort_key);

  return(FALSE); // succeeded in scheduling all -- did not fail

}

void asap_schedule(int initial_width, int initial_height,int cluster_size, int input_cluster_size,
		  int makespan)
{

  boolean schedule_fail=TRUE;
  int width=initial_width;
  int height=initial_height;

  // make sure have created level array first
  if (level==(int *)NULL)
    asap_delay();

  while (schedule_fail)
    {
      allocate_mesh(width, height, cluster_size, input_cluster_size);
      schedule_fail=asap_schedule_on_array(cluster_size,input_cluster_size); 
          // fails if cannot fit ASAP schedule
      if (schedule_fail)
	{
#ifdef VERBOSE_ASAP_SCHEDULE
	  fprintf(stderr,"Failed to schedule on %d x %d mesh; reclaim and increment\n",
		  width,height);
#endif
	  // remove and unschedule old mesh
	  free_mesh();

	  // make mesh larger
	  if (width>height)
	    height++;
	  else
	    width++;
	}
    }
  
  return;
}

