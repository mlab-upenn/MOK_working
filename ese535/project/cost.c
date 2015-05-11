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
  int i;
  int j;
  int *max_io=(int *)malloc(sizeof(int)*height);
  for(i=0; i<height; i++)
    max_io[i]=0;
  for (i=0; i<get_nodes(); i++)
  {
    int in;
    int out;
    int current_height=tree_location[i].height;
    for(j=0; j<tree_location[i].parents; j++)
    {
      in=count_in_domain(tree_location[i].parent_in_domains[j]);
      out=count_in_domain(tree_location[i].parent_out_domains[j]);
      if(in>max_io[current_height])
      {
        max_io[current_height]=in;
      }          
      if(out>max_io[current_height])
      {
        max_io[current_height]=out;
      }
    }
  }
  return(max_io);
}



/* only consider nets driven at specified level */
int *max_io_by_height_at_level(int height, int level)
{
  int i;
  int j;
  int *max_io_level=(int *)malloc(sizeof(int)*height);
  for(i=0; i<height; i++)
    max_io_level[i]=0;
  for (i=0; i<get_nodes(); i++)
  {
    int in;
    int out;
    int current_height=tree_location[i].height;
    for(j=0; j<tree_location[i].parents; j++)
    {
      in=count_in_domain_by_level(level,tree_location[i].parent_in_domains[j]);
      out=count_in_domain_by_level(level,tree_location[i].parent_out_domains[j]);
      if(in>max_io_level[current_height])
      {
        max_io_level[current_height]=in;
      }          
      if(out>max_io_level[current_height])
      {
        max_io_level[current_height]=out;
      }
    }
  }
  return(max_io_level);
}

/* max over all levels */
int *max_io_by_height_any_level(int height, int makespan)
{
  int i;
  int j;
  int *max_io_any_level=(int *)malloc(sizeof(int)*height);
  for(i=0; i<height; i++)
    max_io_any_level[i]=0;

  for(i=0; i<makespan; i++)
  {
    int *temp;
    temp=max_io_by_height_at_level(height, i);
    for (j=0; j<height; j++)
    {
      if(temp[j]>max_io_any_level[j])
      {
        max_io_any_level[j]=temp[j];
      }
    }
  }
  return(max_io_any_level);
}

/* identify minimum cost growth schedule to support traffic indicated */
int *compute_growth_schedule(int height, int *max_io_by_height)
{
  int *g;
  g=(int *)malloc((height-1)*sizeof(int));
  int wires[height-1];
  memset(wires,-1,height-1);
  memset(g,1,height-1);
  int i;
  for (i=0; i<=height-1;i++)
  {
    if (i==0)
    {
      g[i]=max_io_by_height[i];
      wires[i]=g[i];
    }
    else
    {
      int temp_prev=wires[i-1];
      int temp_curr=max_io_by_height[i];
      if (temp_curr>temp_prev)
      {
        g[i]=2;
      }
      else
      {
        g[i]=1;
      }
      wires[i]=temp_prev*g[i];
    }
  }
  #ifdef CONSOLE
  printf("GROWTH SCHEDULE:\n");
  #endif
  for (i=0; i<=height-1; i++)
  {
    #ifdef CONSOLE
    printf("level: %d g: %d\n", i, g[i]);
    #endif
  }
  return(g);

}

/* compute cost given growth schedule */
int cost_of_growth_schedule(int height, int *growth_schedule)
{
  int c_sigs=1;
  int channels=0;
  int length=0;
  int sum=0;
  int exponent=0;
  int temp=0;
  int i;
  int j;
  for(i=0; i<=height-1; i++)
  {
    c_sigs=1.0;
    for (j=0; j<=i; j++)
    {
      c_sigs=growth_schedule[j]*c_sigs;
    }
    exponent=ceil(i/2.0);
    temp=c_sigs*((pow(2,height))/(pow(2,i)))*(pow(2,exponent));
    sum=sum+temp;
  }
  #ifdef CONSOLE
  printf("Interconnect Cost %d\n", sum);
  #endif
  return(sum);

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
