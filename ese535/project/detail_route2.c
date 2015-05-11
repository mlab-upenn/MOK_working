#include <stdio.h>
#include <string.h>
#include "util.h"
#include "main.h"
#include "tree.h"
#include "globals.h"
#include "detail_route.h"
#include "list.h"

void detail_route(boolean *is_clock, boolean global_clock, int maxwaves, boolean verbose)
{
    int i;
    // Allocate memory for timesteps associated with nets
    net_timestep=(int *)malloc(sizeof(int)*num_nets);
    for(i=0; i<num_nets; i++)
    {
        net_timestep[i]=-1;
    }
    printf("net timestep: %d", net_timestep[1]);
    // Sort blocks
    sort_blocks(is_clock,global_clock,FALSE);

    // Route net
    route_all_nets(is_clock,global_clock,FALSE);

    return;
}

void sort_blocks(boolean *is_clock, boolean global_clock, boolean verbose)
{
    // Variables
    int i;
    ListPtr block_list;
    block_list=NULL;
    //block_list->nextPtr=NULL;

    // Placeholder sort
    for(i=0; i<=num_nets; i++)
    {
        if ((global_clock==TRUE) && (is_clock[i]==FALSE)) // don't route clocks
        {
            printf("Inserting item %d, %s in block list\n",i, block[i].name);
            insert(&block_list,i);
            block_list->gain=-1;
            block_list->placement=block[i].loc;
            net_timestep[i]=i;
        }

    }
    printf("Printing the list\n");
    printList(block_list);


}

void route_all_nets(boolean *is_clock, boolean global_clock, boolean verbose)
{
    int i;
    for(i=0; i<num_nets; i++)
    {
        route_net(i,is_clock,global_clock,FALSE);        
    }


}

void route_net(int whichNet, boolean *is_clock, boolean global_clock, boolean verbose)
{
    // Loop counters
    int j;
    int i;
    int current_node;
    // src is the block
    int src=net[whichNet].pins[0];
    // sloc is the PE
    int sloc=block[src].loc;
    // counter for retry
    int retry=0;

    if(is_clock[src]==FALSE)
    {
        for (j=1;j<net[whichNet].num_pins;j++)
        {
          boolean route_outcome;
          int dst=net[whichNet].pins[j];
          int loc=block[dst].loc;
          current_node=src;
          printf("About to route %d to %d\n",src,dst);
          route_outcome=route_source_sink(sloc,loc,sloc,src, dst, whichNet);
          if (route_outcome==FALSE)
          {
            // Go back to start of the for loop and reroute
            j=1;
            // Update counter
            retry++;
            // We should never have to try more than once to reroute net
            if(retry>1)
                return;
          }
        }
    }
}

boolean route_source_sink(int sloc, int loc, int current_node, int src, int dst, int whichNet, int previous_node)
{
    int next_node;
    printf("Global Route %s(%d) %d->%d:\n",net[whichNet].name,whichNet,sloc,loc);
    printf("Current node: %d Source: %d\n", current_node, src);

    if(current_node==loc)
    {
        printf("Reached destination\n\n");
        return TRUE;
    }

    if(tree_location[current_node].left_in_domains[domain]==net)
    {
        return FALSE;
    }


    else
    {
        if(current_node==sloc)
        {
            boolean result;
            next_node=tree_location[current_node].parent[0];
            previous_node=current_node;
            printf("First: next node: %d\n", next_node);
            result=route_source_sink(sloc,loc,next_node,src,dst,whichNet, previous_node);  
            if(result==TRUE)
            {
                return;
            }
            else
            {
                if(tree_location[current_node].parents>1)
                {
                    next_node=tree_location[current_node].parent[1];
                    route_source_sink(sloc,loc,next_node,src,dst,whichNet,previous_node);
                }

                else
                {
                    return FALSE;
                }
            }
        }
        else
        {
            int parent_location=tree_location[current_node].parent[0];
            int parent_addr=tree_location[parent_location].location;
            printf("Parent location: %d, destination %d\n",parent_location,block[dst].loc);
            printf("Left: %d, Right: %d\n",tree_location[current_node].left,tree_location[current_node].right);

            if ((path_to_leaf_side(current_node,sloc)==TRUE) && (path_to_leaf_side(current_node,block[dst].loc)==FALSE))
            {
                next_node=tree_location[current_node].parent[0];
                printf("Case 1: next node: %d\n", next_node);
                // TO DO: Place node in domain...
                route_source_sink(sloc,loc,next_node,src,dst,whichNet);  
            }

            else if (path_to_leaf_side(tree_location[current_node].left,block[dst].loc)==TRUE)
            {
                next_node=tree_location[current_node].left;
                printf("Case 2: next node: %d\n", next_node);
                // TO DO: Place node in domain...
                route_source_sink(sloc,loc,next_node,src,dst,whichNet);
               
            }

            else if (path_to_leaf_side(tree_location[current_node].right,block[dst].loc)==TRUE)
            {
                next_node=tree_location[current_node].right;
                printf("Case 3: next node: %d\n", next_node);
                // TO DO: Place node in domain...
                route_source_sink(sloc,loc,next_node,src,dst,whichNet);
                
            }
        }
    }   

}






