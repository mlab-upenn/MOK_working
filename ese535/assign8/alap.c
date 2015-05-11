
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "tree.h"
#include "main.h"
#include "globals.h"
#include "asap.h"
#include "queue.h"

#define UNSCHED_SUCC -1
//#define DEBUG_ALAP

int *alap_level=(int *)NULL;
int *alap_sort=(int *)NULL;

void process_ready_queue(int *ready_queue, int *alap_level, int current_timestep)
{
  int unsc_counter=0;
  static int rec_counter;

    while(empty_queue(ready_queue)==FALSE)
    {
      int next_block=pop_queue(ready_queue);

      if (block[next_block].type==INPAD || block[next_block].type==LUT_AND_LATCH)
      {
        alap_level[next_block]=0;
      }
      else if(alap_level[next_block]==LEVEL_UNDETERMINED)
      {
        #ifdef DEBUG_ALAP
        printf("about to check next block: %s\n", block[next_block].name);
        #endif
        int ready=all_succ_sched(next_block, current_timestep, alap_level);
        if(ready !=UNSCHED_SUCC)
        {
          #ifdef DEBUG_ALAP
          printf("Ready: %d\n", ready);
          #endif
          alap_level[next_block]=ready-1;

        }
        else
        {
          unsc_counter++;
        }
      }
    }

    if(unsc_counter==0)
    {
      return;
    }

    else if(unsc_counter!=0 && rec_counter<5)
    {
      rec_counter++;
      unsc_counter=0;
      reset_queue(ready_queue);
      process_ready_queue(ready_queue, alap_level, current_timestep);
    }
}

//int *alap_delay(boolean *is_clock)
void alap_delay(void)
{
  int i;
  int j;
  int k;
  int current_timestep=alap_time_max;
  //printf("ASAP Max Timestep: %d\n", asap_delay());

// setup alap levels array
  alap_level = (int *)my_malloc(sizeof(int)*(num_blocks));
  alap_sort = (int *)my_malloc(sizeof(int)*(num_blocks));
// setup ready queue
  int *ready_queue=new_queue(num_blocks);

// initialize levels
  for (i=0;i<num_blocks+1;i++)
    if (block[i].type==OUTPAD)
    {
      alap_level[i]=current_timestep;
    }
    
    else
      alap_level[i]=LEVEL_UNDETERMINED;

// see what's ready now and fill ready queue
    for (i=0;i<num_blocks;i++)
    {

      if (block[i].type==OUTPAD)
      {
    // skip already processed
      }

      else 
      {
        if(alap_level[i]==LEVEL_UNDETERMINED && block[i].loc!=GLOBAL_CLOCK_LOC)
          push_queue(ready_queue,i);
      }
    }

    process_ready_queue(ready_queue,alap_level, current_timestep);

    free_queue(ready_queue);
    my_heapsort(alap_sort,alap_level,num_blocks+1);
    for(i=0; i<num_blocks; i++)
    {
      
      printf("ALAP LEVEL SORT: %s, %d\n", block[alap_sort[i]].name, alap_level[alap_sort[i]]);
      
    }

    //return alap_sort;
  }

int all_succ_sched(int whichBlock, int current_timestep, int *alap_level, int *is_clock)
{
  int i;
  int blevel;
  int local_time=current_timestep;
  #ifdef DEBUG_ALAP
  printf("GOT HERE, num pins: %d\n",net[block[whichBlock].nets[0]].num_pins);
  #endif
   int start=1;
    if (block[whichBlock].type==OUTPAD)
        start=0;

  for(i=start;i<net[block[whichBlock].nets[0]].num_pins;i++)
  {
    int successor_block=net[block[whichBlock].nets[0]].pins[i];
    int blevel=alap_level[successor_block];
    #ifdef DEBUG_ALAP
    printf("Processing %s (%d) at level %d.\n", block[successor_block].name, successor_block,blevel);
    #endif
    if(block[successor_block].type==LUT_AND_LATCH)
    {
      return alap_time_max;
    }
    if (blevel==LEVEL_UNDETERMINED)
    {
      return UNSCHED_SUCC;
    }
    else
    {
      if(blevel<local_time)
      {
        local_time=blevel;
      }
    }
  }
  return local_time;
}



