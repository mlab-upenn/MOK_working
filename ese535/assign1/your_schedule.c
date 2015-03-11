#include <stdio.h>
#include <string.h>
#include "util.h"
#include "mesh.h"
#include "sched_main.h"
#include "globals.h"


void your_schedule(int initial_width, int initial_height,int cluster_size, int input_cluster_size,
		  int makespan)
{
  int width=initial_width;
  int height=initial_height;


  /* TODO: your scheduler code goes here */

  /* will need allocate array */
  /*   but you may need to determine what width and height should be */
  /* following is a place holder */
  allocate_mesh(width, height, cluster_size, input_cluster_size);

  /* TODO: assign each block to a timestep/slot/PE(x,y) */

  return;
}

