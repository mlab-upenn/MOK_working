/* routines to compute cost input and costs for assign2 (and probably used for assign 3-6) */
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "tree.h"
#include "domain.h"
#include "cost.h"


/* #define OPT_LEVEL */

/* max over all subtrees (switches for assign2) at given height */
int *max_io_by_height(int height)
{
  /* FILLIN */
  return((int *)NULL);
}

/* only consider nets driven at specified level */
int *max_io_by_height_at_level(int height, int level)
{
  /* FILLIN */
  return((int *)NULL);
}

/* max over all levels */
int *max_io_by_height_any_level(int height, int makespan)
{
  /* FILLIN */
  return((int *)NULL);
}

/* identify minimum cost growth schedule to support traffic indicated */
int *compute_growth_schedule(int height, int *max_io_by_height)
{
  /* FILLIN */
  return((int *)NULL);
}

/* compute cost given growth schedule */
int cost_of_growth_schedule(int height, int *growth_schedule)
{
  /* FILLIN */
  return(1<<30);
}

void calculate_and_print_costs(char *blif_file, char *cost_file, int makespan, int height)
{

 FILE *fp; 
 int i;

 fp = my_fopen(cost_file,"w",0);

 int *max_io=max_io_by_height(height);
 int *level_max_io=max_io_by_height_any_level(height,makespan);
 int *grow_single=compute_growth_schedule(height,max_io);
 int *grow_level=compute_growth_schedule(height,level_max_io);
 int interconnect_cost=cost_of_growth_schedule(height,grow_single);
 int level_cost=cost_of_growth_schedule(height,grow_level);
 int total_level_cost=level_cost*makespan;
 float context_factor=(float)total_level_cost/(float)interconnect_cost;



 fprintf(fp,"single growth:");
 if (grow_single!=(int *)NULL)
   for (i=0;i<height;i++)
     fprintf(fp,"%d ",grow_single[i]);
 fprintf(fp,"\n");

 fprintf(fp,"level growth:");
 if (grow_level!=(int *)NULL)
   for (i=0;i<height;i++)
     fprintf(fp,"%d ",grow_level[i]);
 fprintf(fp,"\n");

 fprintf(fp,"%s, %d, %d, %d, %3.2f\n",blif_file,makespan,interconnect_cost,level_cost,context_factor);

#ifdef OPT_LEVEL 

 int *opt_level_factor=(int *)NULL;
 int *opt_grow_level=(int *)NULL;

 /* FILLIN your code here as appropriate */
 
 int opt_total_level_cost=-1/* FILLIN */;
 float opt_context_factor=(float)opt_total_level_cost/(float)interconnect_cost;
 int opt_level_cost=-1/* FILLIN */;

 fprintf(fp,"level factor:");
 if (opt_level_factor!=(int *)NULL)
   for (i=0;i<makespan;i++)
     fprintf(fp,"%d ",opt_level_factor[i]);
 fprintf(fp,"\n");

 fprintf(fp,"level growth:");
 if (opt_grow_level!=(int *)NULL)
   for (i=0;i<height;i++)
     fprintf(fp,"%d ",opt_grow_level[i]);
 fprintf(fp,"\n");

 fprintf(fp,"%s, %d, %d, %d, %3.2f\n",blif_file,makespan,interconnect_cost,opt_level_cost,opt_context_factor);

 //free(opt_level_factor);
 //free(opt_grow_level);

#endif

 fclose(fp);

}
