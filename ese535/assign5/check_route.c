/* check if routes are valid */
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "util.h"
#include "domain.h"
#include "tree.h"
#include "globals.h"
#include "check_route.h"

/* count violations for all routes */
int valid_routes(boolean *is_clock, boolean global_clock, boolean verbose)
{
  int i,j;
  int failures=0;
  int limit=ilog2(get_size())*2;

  for (i=0;i<num_nets;i++)
    {
      if ((global_clock==TRUE) && (is_clock[i]==FALSE)) // don't expected clocks to be routed
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
		    printf("Route net=%s from %d to %d:",net[i].name,sloc,loc);
		  if (valid_route(i,sloc,loc,verbose,limit)==FALSE)
		    {
		      failures++;
		      if (verbose==TRUE)
			printf(" BAD");
		    }
		  if (verbose==TRUE)
		    printf("\n");
		}
	    }
	}
    }

  return(failures);
  
}

/* check a single route */
boolean valid_route(int net, int current_node, int dst, boolean verbose, int limit)
{

  int j;

  if (verbose==TRUE) 
    printf(" %d(h=%d,a=%d)",current_node,tree_location[current_node].height,tree_location[current_node].location);

  if (limit<0)
    {
      fprintf(stderr,"valid_route exceeded limit\n");
      dump_parent_domains("debug.bad.rinfo");
      exit(100);
    }
  

  if (current_node==dst)
    return(TRUE);

  if ((tree_location[current_node].parents>0) && (path_to_leaf_side(current_node,dst)==FALSE))
    {
      // due to fanout, the net could be routed up and crossover;
      //  test above checks if this path should be going up.
      for (j=0;j<tree_location[current_node].parents;j++)
	if (in_domain(net,tree_location[current_node].parent_out_domains[j]))
	  {
	    // check appears on appropriate input domain
	    if (tree_location[current_node].parent_side==LEFT)
	      if (in_domain(net,tree_location[tree_location[current_node].parent[j]].left_in_domains))
		return(valid_route(net,tree_location[current_node].parent[j],dst,verbose,limit-1));
	      else
		return(FALSE);
	    if (in_domain(net,tree_location[tree_location[current_node].parent[j]].right_in_domains))
	      return(valid_route(net,tree_location[current_node].parent[j],dst,verbose,limit-1));
	    else
	      return(FALSE);
	  }
    }

  int next_node=-1;

  /* note: with fanout, it's posible a net is routed both to the left and to the right
     ...so need to pay attention to the destination to disambiguate which one to follow */

  if ((in_domain(net,tree_location[current_node].left_out_domains)==TRUE) && (path_to_leaf_side(tree_location[current_node].left,dst)))
    {
      next_node=tree_location[current_node].left;
    }

  if ((in_domain(net,tree_location[current_node].right_out_domains)==TRUE) && (path_to_leaf_side(tree_location[current_node].right,dst)))
    {
      next_node=tree_location[current_node].right;
    }

  if (next_node>=0)
    {
      for (j=0;j<tree_location[next_node].parents;j++)
	if (in_domain(net,tree_location[next_node].parent_in_domains[j]))
	  if (tree_location[next_node].parent[j]==current_node) // and make sure it's the right parent
	    return(valid_route(net,next_node,dst,verbose,limit-1));
      return(FALSE);
    }
  else
    return(FALSE);


}

