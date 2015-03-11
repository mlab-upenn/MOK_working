/* routines to compute cost input and costs for assign2 (and probably used for assign 3-6) */
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "tree.h"
#include "domain.h"
#include "cost.h"

//#define DEBUG

/* #define OPT_LEVEL */


/* max over all subtrees (switches for assign2) at given height */
int *max_io_by_height(int height)
{
  int incoming=0;
  int outgoing=0;

  // Compute the maximum height
  int max_height= log2(get_size());
  //printf("%d\n", max_height);
  // Initialize max_io
  static int height_max_io[10000];
  memset(height_max_io,-1,10000);
  // Create an array to store the parents of a particular height
  int parents_for_nav[max_height][get_size()];
  // Initialize the array with -1's in case we don't fill the entire array
  memset(parents_for_nav,-1,get_size());

  // For loop which iterates through all the PEs
  // Determines parents and height=0 maximum io
  for (int i=0; i<get_size(); i++)
  {
    // Array to store the maximum io at each node
    int node_max[get_size()];
    if( i%2 ==0)
    {
      incoming =count_in_domain( tree_location[*tree_location[i].parent].left_in_domains);
      outgoing =count_in_domain( tree_location[*tree_location[i].parent].left_out_domains);
    }
    else
    {
      incoming =count_in_domain( tree_location[*tree_location[i].parent].right_in_domains);
      outgoing =count_in_domain( tree_location[*tree_location[i].parent].right_out_domains);
    }
    parents_for_nav[0][i]=*tree_location[i].parent;

    #ifdef DEBUG
    printf("parent of PE%d is Switch %d\n", i, parents_for_nav[0][i]);
    #endif

    if (incoming>=outgoing)
    {
      node_max[i]=incoming;
    }
    else
    {
      node_max[i]=outgoing;
    }
    // At each node check if we need to update the height maximum
    if (node_max[i]>height_max_io[0] && node_max[i]<32767)
    {
      height_max_io[0]=node_max[i];
    }
    #ifdef DEBUG
    printf("Incoming to PE%d : %d\n", i, incoming);
    printf("Outgoing from PE%d : %d\n", i, outgoing);
    printf("Maximum at PE%d : %d\n", i, node_max[i]);
    #endif
  }

  for (int h=1; h<=max_height-1; h++)
  {
    for (int i=0; i<=get_size(); i++)
    {
      int node_max[get_size()];
      memset(node_max,0,get_size());
      static int incoming=0;
      static int outgoing=0;
      if(tree_location[parents_for_nav[h-1][i]].parents !=0)
      {
        if( i%2 ==0)
        {
          incoming =count_in_domain( tree_location[*tree_location[parents_for_nav[h-1][i]].parent].left_in_domains);
          outgoing =count_in_domain( tree_location[*tree_location[parents_for_nav[h-1][i]].parent].left_out_domains);
        }
        else
        {
          incoming =count_in_domain( tree_location[*tree_location[parents_for_nav[h-1][i]].parent].right_in_domains);
          outgoing =count_in_domain( tree_location[*tree_location[parents_for_nav[h-1][i]].parent].right_out_domains);
        }
        parents_for_nav[h][i]=*tree_location[parents_for_nav[h-1][i]].parent;
        #ifdef DEBUG
        printf("parent of Switch %d is Switch %d\n", parents_for_nav[h-1][i],parents_for_nav[h][i] );
        #endif
        if (incoming>=outgoing)
        {
          node_max[i]=incoming;
        }
        else
        {
          node_max[i]=outgoing;
        }
      // At each node check if we need to update the height maximum
        #ifdef DEBUG
        printf("%d, %d\n", node_max[i], height_max_io[h]);
        #endif
        if (node_max[i]>height_max_io[h] && node_max[i]<32767)
        {
          height_max_io[h]=node_max[i];
          #ifdef DEBUG
          printf("UPDATE: Max height %d\n",height_max_io[h]);
          #endif
        }
      }
      #ifdef DEBUG
      printf("Incoming to Switch %d : %d\n", parents_for_nav[h-1][i], incoming);
      printf("Outgoing from Switch %d : %d\n", parents_for_nav[h-1][i], outgoing);
      printf("Maximum at Switch %d : %d\n", parents_for_nav[h-1][i], node_max[i]);
      printf("height= %d\n", h);
      #endif

    }
  }

  printf("FOR ALL LEVELS:\n");

  for (int j=0; j<max_height; j++)
  {
    printf("Maximum flow at height %d %d\n", j, height_max_io[j]);
  }
  printf("Max Height %d\n", max_height);
  return((int *) height_max_io);
}

/* only consider nets driven at specified level */
int *max_io_by_height_at_level(int height, int level)
{
  int incoming=0;
  int outgoing=0;
  for(int l=0; l<level; l++)
  {
  // Compute the maximum height
    int max_height= log2(get_size());
  // Initialize max_io
    int *height_max_io;
    height_max_io=(int *)malloc(10000*sizeof(int));
    memset(height_max_io,-1,10000);
  // Create an array to store the parents of a particular height
    int parents_for_nav[max_height][get_size()];
  // Initialize the array with -1's in case we don't fill the entire array
    memset(parents_for_nav,-1,get_size());
  // Create a static variable for the current node.
    static int current_node;

  // For loop which iterates through all the PEs
  // Determines parents and height=0 maximum io
    for (int i=0; i<get_size(); i++)
    {
    // Array to store the maximum io at each node
      int node_max[get_size()];
      if( i%2 ==0)
      {
        incoming =count_in_domain_by_level(0, tree_location[*tree_location[i].parent].left_in_domains);
        outgoing =count_in_domain_by_level(0, tree_location[*tree_location[i].parent].left_out_domains);
      }
      else
      {
        incoming =count_in_domain_by_level(0, tree_location[*tree_location[i].parent].right_in_domains);
        outgoing =count_in_domain_by_level(0, tree_location[*tree_location[i].parent].right_out_domains);
      }
      parents_for_nav[0][i]=*tree_location[i].parent;

      #ifdef DEBUG
      printf("parent of PE%d is Switch %d\n", i, parents_for_nav[0][i]);
      #endif

      if (incoming>=outgoing)
      {
        node_max[i]=incoming;
      }
      else
      {
        node_max[i]=outgoing;
      }
      // At each node check if we need to update the height maximum
      if (node_max[i]>height_max_io[0] && node_max[i]<32767)
      {
        height_max_io[0]=node_max[i];
      }
      #ifdef DEBUG
      printf("Incoming to PE%d : %d\n", i, incoming);
      printf("Outgoing from PE%d : %d\n", i, outgoing);
      printf("Maximum at PE%d : %d\n", i, node_max[i]);
      #endif
    }

    for (int h=1; h<=max_height-1; h++)
    {
      for (int i=0; i<=get_size(); i++)
      {
        int node_max[get_size()];
        memset(node_max,0,get_size());
        static int incoming=0;
        static int outgoing=0;
        if(tree_location[parents_for_nav[h-1][i]].parents !=0)
        {
          if( i%2 ==0)
          {
            incoming =count_in_domain_by_level(l, tree_location[*tree_location[parents_for_nav[h-1][i]].parent].left_in_domains);
            outgoing =count_in_domain_by_level(l, tree_location[*tree_location[parents_for_nav[h-1][i]].parent].left_out_domains);
          }
          else
          {
            incoming =count_in_domain_by_level(l, tree_location[*tree_location[parents_for_nav[h-1][i]].parent].right_in_domains);
            outgoing =count_in_domain_by_level(l, tree_location[*tree_location[parents_for_nav[h-1][i]].parent].right_out_domains);
          }
          parents_for_nav[h][i]=*tree_location[parents_for_nav[h-1][i]].parent;

          #ifdef DEBUG
          printf("parent of Switch %d is Switch %d\n", parents_for_nav[h-1][i],parents_for_nav[h][i] );
          #endif

          if (incoming>=outgoing)
          {
            node_max[i]=incoming;
          }
          else
          {
            node_max[i]=outgoing;
          }
      // At each node check if we need to update the height maximum
          #ifdef DEBUG
          printf("%d, %d\n", node_max[i], height_max_io[h]);
          #endif

          if (node_max[i]>height_max_io[h] && node_max[i]<32767)
          {
            height_max_io[h]=node_max[i];

            #ifdef DEBUG
            printf("UPDATE: Max height %d\n",height_max_io[h]);
            #endif
          }
        }
        #ifdef DEBUG
        printf("Incoming to Switch %d : %d\n", parents_for_nav[h-1][i], incoming);
        printf("Outgoing from Switch %d : %d\n", parents_for_nav[h-1][i], outgoing);
        printf("Maximum at Switch %d : %d\n", parents_for_nav[h-1][i], node_max[i]);
        printf("height= %d\n", h);
        #endif

      }
    }


    printf("FOR LEVEL %d\n", l);

    for (int j=0; j<max_height; j++)
    {
      printf("Maximum flow at height %d %d\n", j, height_max_io[j]);
    }
    printf("Makespan %d\n", level);

    if (l == level)
    {
      return((int *) height_max_io);
    } 
  }
}

/* max over all levels */
int *max_io_by_height_any_level(int height, int makespan)
{
  
  int max_any_level=0;
  for (int i=0; i<=makespan-1;i++)
  {
    printf("BEFORE THE CRASH\n");
    int *temp_level= max_io_by_height_at_level(height-1,i);
    printf("temp_level: %d\n", temp_level[1]);
    /*if (temp_level>=max_any_level)
    {
      max_any_level=temp_level;
    }*/
  }

  //return((int*) max_any_level);
}

/* identify minimum cost growth schedule to support traffic indicated */
int *compute_growth_schedule(int height, int *max_io_by_height)
{
  int g[height-1];
  int wires[height-1];
  memset(wires,-1,height-1);
  memset(g,1,height-1);
  for (int i=0; i<=height-1;i++)
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
  printf("GROWTH SCHEDULE:\n");
  for (int i=0; i<=height-1; i++)
  {
    printf("level: %d g: %d\n", i, g[i]);
  }

  /* FILLIN */
  return((int *)g);
}

/* compute cost given growth schedule */
int cost_of_growth_schedule(int height, int *growth_schedule)
{
  for(int i=0; i<=height-1; i++)
  {
    
  }
  /* FILLIN */
  return(1<<30);
}

void calculate_and_print_costs(char *blif_file, char *cost_file, int makespan, int height)
{

 FILE *fp; 
 int i;

 fp = my_fopen(cost_file,"w",0);

 int *max_io=max_io_by_height(height);
 int *test=max_io_by_height_at_level(height,makespan);
 printf("Got here\n");
 int *level_max_io=max_io_by_height_any_level(height,makespan);
 printf("Got here");
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
