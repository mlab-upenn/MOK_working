/* for performing global routes for assign2 (and probably 3-6) */
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "main.h"
#include "domain.h"
#include "tree.h"
#include "globals.h"
#include "global_route.h"

#define DEBUG_GLOBAL_ROUTE FALSE

/* route all non-local nets */
void global_route(boolean verbose)
{


  int i,j;
  for (i=0;i<num_nets;i++)
    {
      int src=net[i].pins[0];
      int sloc=block[src].loc;
      for (j=1;j<net[i].num_pins;j++)
	{
	  int dst=net[i].pins[j];
	  int loc=block[dst].loc;
	  if (sloc!=loc) /* skipping self routes for now */
	    {
	      if (verbose==TRUE)
		printf("Global Route %s(%d) %d->%d:",net[i].name,i,sloc,loc);
	      route(i,sloc,sloc,loc,verbose);
	      if (verbose==TRUE)
		printf("\n");
	      if (DEBUG_GLOBAL_ROUTE)
		if (valid_route(i,sloc,loc,TRUE,100)==FALSE)
		  {
		    printf("\n");
		    fflush(stdout);
		    fprintf(stderr,"ROUTE INSERT FAIL\n");
		    fflush(stderr);
		    dump_parent_domains("debug.bad.rinfo");
		    exit(101);
		  }
		else
		  {
		    printf("ROUTE INSERT OK\n");
		    dump_parent_domains("debug.good.rinfo");
		  }
	    }
	}
    }
}

/* route a single two-point net (one source to one destination) */
void route(int which_net, int current_node, int src, int dst,boolean verbose)
{

  if (verbose==TRUE) 
    printf(" %s %d(h=%d,a=%d)",net[which_net].name,current_node,tree_location[current_node].height,tree_location[current_node].location);


  /* where come from? */
  if (src==current_node)
    {
      // just starting here, nothing to add
    }
  else if ((tree_location[current_node].parents>0) && (path_to_leaf_side(current_node,src)==FALSE))
    {
      // came from a parent
      // for global, only using parent 0
      tree_location[current_node].parent_in_domains[0]=unique_add_domain(which_net,tree_location[current_node].parent_in_domains[0]);
    }
  else if (path_to_leaf_side(tree_location[current_node].left,src)==TRUE)
    {
      // came from left
      tree_location[current_node].left_in_domains=unique_add_domain(which_net,tree_location[current_node].left_in_domains);
    }
  else if (path_to_leaf_side(tree_location[current_node].right,src)==TRUE)
    {
      // came from right
      tree_location[current_node].right_in_domains=unique_add_domain(which_net,tree_location[current_node].right_in_domains);
    }
  else
    {
      fprintf(stderr,"route: failed to identify where came from for net=%d src=%d dst=%d at node %d\n",
	      which_net,src,dst,current_node);
    }
    

  /* is it there, yet? */
  if (current_node==dst)
    return;

  /* where going? */
  if ((path_to_leaf_side(current_node,src)==TRUE) && (path_to_leaf_side(current_node,dst)==FALSE))
    {
      // still need to route higher in tree
      // for global, only using parent 0
      tree_location[current_node].parent_out_domains[0]=unique_add_domain(which_net,tree_location[current_node].parent_out_domains[0]);
      route(which_net,tree_location[current_node].parent[0],src,dst,verbose);
    }
  else if (path_to_leaf_side(tree_location[current_node].left,dst)==TRUE)
    {
      // need to route left
      tree_location[current_node].left_out_domains=unique_add_domain(which_net,tree_location[current_node].left_out_domains);
      route(which_net,tree_location[current_node].left,src,dst,verbose);
    }
  else
    {
      // route right
      tree_location[current_node].right_out_domains=unique_add_domain(which_net,tree_location[current_node].right_out_domains);
      route(which_net,tree_location[current_node].right,src,dst,verbose);
    }
  
} 
