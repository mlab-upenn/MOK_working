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
	  int start=1;
	  if (block[src].type==OUTPAD)
	    start=0;
	  for (j=start;j<net[i].num_pins;j++)
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


/* check a single route on a single domain */
boolean valid_detail_route_domain(int net, int domain, int current_node, int dst, boolean verbose, int limit)
{

  int j;

  if (verbose==TRUE) 
    printf(" %d(h=%d,a=%d)",current_node,tree_location[current_node].height,tree_location[current_node].location);

  if (limit<0)
    {
      fprintf(stderr,"valid_route exceeded limit\n");
      dump_parent_domains("debug.bad.rinfo");
      exit(179);
    }
  

  if (current_node==dst)
    return(TRUE);

  if ((tree_location[current_node].parents>0) && (path_to_leaf_side(current_node,dst)==FALSE))
    {
      // due to fanout, the net could be routed up and crossover;
      //  test above checks if this path should be going up.
      for (j=0;j<tree_location[current_node].parents;j++)
	if (tree_location[current_node].parent_out_domains[j][domain]==net)
	  {
	    // check appears on appropriate input domain
	    if (tree_location[current_node].parent_side==LEFT)
	      {
		if (tree_location[tree_location[current_node].parent[j]].left_in_domains[domain]==net)
		  if (valid_detail_route_domain(net,domain,tree_location[current_node].parent[j],dst,verbose,limit-1)==TRUE)
		    return(TRUE);
	      }
	    else // right
	      {
		if (tree_location[tree_location[current_node].parent[j]].right_in_domains[domain]==net)
		  if (valid_detail_route_domain(net,domain,tree_location[current_node].parent[j],dst,verbose,limit-1)==TRUE)
		    return(TRUE);
	      }
	  }
      return(FALSE);
    }

  int next_node=-1;

  /* note: with fanout, it's posible a net is routed both to the left and to the right
     ...so need to pay attention to the destination to disambiguate which one to follow */

  if ((tree_location[current_node].left_out_domains[domain]==net) && (path_to_leaf_side(tree_location[current_node].left,dst)))
    {
      next_node=tree_location[current_node].left;
    }

  if ((tree_location[current_node].right_out_domains[domain]==net) && (path_to_leaf_side(tree_location[current_node].right,dst)))
    {
      next_node=tree_location[current_node].right;
    }

  if (next_node>=0)
    {
      for (j=0;j<tree_location[next_node].parents;j++)
	if (tree_location[next_node].parent_in_domains[j][domain]==net)
	  if (tree_location[next_node].parent[j]==current_node) // and make sure it's the right parent
	    return(valid_detail_route_domain(net,domain,next_node,dst,verbose,limit-1));
      return(FALSE);
    }
  else
    return(FALSE);


}

boolean valid_detail_route(int which_net, int current_node, int dst, boolean verbose, int limit, int maxwave)
{

  int src_block=net[which_net].pins[0];

  if (block[src_block].loc==GLOBAL_CLOCK_LOC) // in this case clock has dedicated net, not our problem
    return(TRUE); 

  // NOTE: this implicitly assumes there is only one wave on which the source block is routed.
  int d=timestep(src_block);
  printf("d= %d, src= %d\n", d, src_block);
  if (valid_detail_route_domain(which_net,d,current_node,dst,verbose,limit)==TRUE)
    return(TRUE);
  return(FALSE);
  
}

int check_routing_precedence(boolean verbose)
{
    verbose=TRUE;

  int i,j;
  int res=0;
  for (i=0;i<num_blocks;i++)
    {
      int start=1;
      if (block[i].type==OUTPAD)
	start=0;
      for (j=start;j<block[i].num_nets;j++)
	{
	  boolean valid=TRUE;
	  if (block[i].nets[j]!=UNDEFINED)
	    {
	      int which_net=block[i].nets[j];
	      int pred=net[which_net].pins[0];
	      int pred_timestep=timestep(pred);
          if(i==244)
          {
            printf("pred: %s, pred_timestep: %d, j: %d\n", block[pred].name, pred_timestep, j);
          }
	      if (block[i].type==OUTPAD)
		if (timestep(pred)<0) // don't worry about timestep of output pad itself
		  {
		    if (verbose==TRUE)
		      fprintf(stderr,"Output Pad Block %s(%d) predecessor %s(%d) unrouted.\n",
			      block[i].name,i,block[pred].name,pred);
		    valid=FALSE;
		  }
		else
		  {
		    // ok, predecessor was routed
		  }
	      else // not an output pad -- need to check timestep of block relative to predecessor
		if ((block[i].type==LATCH)||
		    (block[i].type==LUT_AND_LATCH))
		  {
		    if (timestep(pred)<0) // don't worry about timestep of output pad itself
		      // FUTURE:  we might actually want to guarantee the LATCH is routed BEFORE any of the predecessors arrive
		      //   but not enforcing that for assignment 7 and 8
		      {
			if (verbose==TRUE)
			  fprintf(stderr,"(LUT and) Latch  Block %s(%d) predecessor %s(%d) unrouted.\n",
				  block[i].name,i,block[pred].name,pred);
			valid=FALSE;
		      }
		    else
		      {
			// ok, predecessor was routed
		      }
		  }
		else  // input (who has no predecessors) or LUT
		  if (timestep(pred)>=timestep(i))
		  {
		    if (verbose==TRUE)
		      if (timestep(pred)==UNDEFINED)
			fprintf(stderr,"Block %s(%d) routed in domain %d, but  predecessor %s(%d) unrouted.\n",
				block[i].name,i,timestep(i),
				block[pred].name,pred);
		      else
			fprintf(stderr,"Block %s(%d) routed in domain %d, which does not follow predecessor %s(%d) routed in domain %d\n",
				block[i].name,i,timestep(i),
				block[pred].name,pred,timestep(pred));
		    valid=FALSE;
		  }
	    }

	  if (valid==FALSE)
	      res++;
	  
	}
      
    }
  return(res);
}

/* count violations for all routes */
int valid_detail_routes(boolean *is_clock, boolean global_clock,  int maxwave, boolean verbose)
{
verbose=TRUE;
  int i,j;
  int failures=0;
  int limit=ilog2(get_size())*2;

  for (i=0;i<num_nets;i++)
    {
      if ((global_clock==TRUE) && (is_clock[i]==FALSE)) // don't expected clocks to be routed
	{
	  int src=net[i].pins[0];
	  int sloc=block[src].loc;
	  int start=1;
	  if (block[src].type==OUTPAD)
	    start=0;
	  for (j=start;j<net[i].num_pins;j++)
	    {
	      int dst=net[i].pins[j];
	      int loc=block[dst].loc;
	      if (sloc!=loc) /* skipping self routes for now */
		{
		  if (verbose==TRUE)
		    printf("Route net=%s from %d to %d:",net[i].name,sloc,loc);
		  if (valid_detail_route(i,sloc,loc,verbose,limit,maxwave)==FALSE)
		    {
		      failures++;
		      if (verbose==TRUE)
			printf(" BAD\n");
		    }
		  if (verbose==TRUE)
		    printf("\n");
		}
	    }
	}
    }


  failures+=check_routing_precedence(verbose);

  return(failures);

  

}
