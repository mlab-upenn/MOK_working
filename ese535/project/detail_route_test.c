#include <stdio.h>
#include <string.h>
#include "util.h"
#include "main.h"
#include "tree.h"
#include "globals.h"
#include "detail_route.h"
#include "list.h"
#include "domain.h"

#define INITIAL_CALL -1
/* #define PARENT_ROUTE 0
#define LEFT_CHILD_ROUTE 1
#define RIGHT_CHILD_ROUTE 2
#define SLOT_USED 3
#define ALREADY_ROUTED 4
#define SLOT_EMPTY 5 */
#define PARENT_OUT_0 0
#define PARENT_OUT_1 1
#define PARENT_IN_0 2
#define PARENT_IN_1 3
#define LEFT_IN 4
#define RIGHT_IN 5
#define LEFT_OUT 6
#define RIGHT_OUT 7
#define DOMAIN_UNUSED_0 -2
#define DOMAIN_UNUSED_1 -3
#define DOMAIN_USED -4

void detail_route(boolean *is_clock, boolean global_clock, int maxwaves, boolean verbose)
{
    int i;

    // Allocate memory for timesteps associated with nets
    net_timestep=(int *)malloc(sizeof(int)*num_nets);
    for(i=0; i<num_nets; i++)
    {
        net_timestep[i]=-1;
    }
    printf("net timestep: %d\n", net_timestep[1]);

    // Sort blocks
    sort_blocks(is_clock,global_clock,FALSE);

    // Route net
    route_all_nets(is_clock,global_clock,FALSE, alap_sort);

    return;
}

void sort_blocks(boolean *is_clock, boolean global_clock, boolean verbose)
{
    // Variables
    int i;
    ListPtr block_list;
    block_list=NULL;
    alap_delay();
    printf("Max timestep: %d\n",alap_time_max);
}

void route_all_nets(boolean *is_clock, boolean global_clock, boolean verbose, int *alap_sort)
{
    int i;
    for(i=num_blocks; i>-1; i--)
    {
        int whichNet=block[alap_sort[i]].nets[0];
        route_net(whichNet,is_clock,global_clock,FALSE);        
    }


}

void route_net(int whichNet, boolean *is_clock, boolean global_clock, boolean verbose)
{
    // Loop counters
    int j;
    int i;
    // Store the current node
    int current_node;
    // src is the block
    int src=net[whichNet].pins[0];
    // sloc is the PE
    int sloc=block[src].loc;
    // reset local timestep
    int local_timestep=0;
    // the minimum step intially can't be worse than alap_time_max
    int min_step=alap_time_max;
    // compute the height of the tree
    int height=get_nodes();
    // the maximum path length can't be more than 2x height
    int max_path=2*ilog2(height);
    // print the maximum path
    printf("Maximum Path: %d\n", max_path);

    // for source nodes that are not clocks compute min step
    if(is_clock[src]==FALSE)
    {
        for (j=1;j<net[whichNet].num_pins;j++)
        {
            int temp=timestep(net[whichNet].pins[j]);
            printf("TEMP: %d\n",temp);
            if(temp<min_step)
            {
                min_step=temp;
            }
        }
    }

    // if the minimum step is unassigned we are dealing with an input block
    if(min_step==UNASSIGNED_TIMESTEP)
    {
        local_timestep=0;
    }

    // otherwise assign the min step value
    else
    {
        local_timestep=min_step;
    }

    // create a pointer for the route array
    //int **route_array;
    ListPtr *route_array;

    // again, if the block is not a clock we need to allocate space for its path
    // initialize this path with -1
    if(is_clock[src]==FALSE)
    {
        route_array=malloc(net[whichNet].num_pins*sizeof(ListPtr));
        for(i=0; i<net[whichNet].num_pins; i++)
        {
            route_array[i]= malloc(max_path*2*sizeof(ListNode));
            insert_path(route_array[i],-1,-1);
        }
        /*
        for(i=0; i<net[whichNet].num_pins; i++)
        {
            for(j=0; j<max_path*2; j++)
            {
                route_array[i][j]=malloc(max_path*2*sizeof(ListPtr);
            }

        }*/

    }

    // now that space has been allocated for a path
    // if the src is not a clock
    // attempt to compute the detailed route
    // if fail, increase local timestep
    if(is_clock[src]==FALSE)
    {
        printf("this is not a clock\n");
        for (j=1;j<net[whichNet].num_pins;j++)
        {
          boolean route_outcome;
          int dst=net[whichNet].pins[j];
          int loc=block[dst].loc;
          current_node=src;
          printf("About to route %d to %d\n",src,dst);
          route_outcome=route_source_sink(sloc,loc,sloc, sloc, src, dst, whichNet, j, local_timestep, 0, route_array);
          if (route_outcome==FALSE)
          {
            // Go back to start of the for loop and reroute
            j=1;
            local_timestep++;
            return;
          }
        }
        // if all the fan out can be routed insert the routes
        insert_route(whichNet, local_timestep, route_array, max_path*2);
    }
}


boolean route_source_sink(int sloc, int loc, int current_node, int previous_node, int src, int dst, 
                          int whichNet, int route_num, int local_timestep, int step_num, ListPtr *route_array)
{
    int next_node;
    int result;
    printf("Global Route %s(%d) %d->%d:\n",net[whichNet].name,whichNet,sloc,loc);
    printf("Current node: %d Source: %d\n", current_node, src);

    if(current_node==loc)
    {
        printf("Reached destination\n\n");
        return TRUE;
    }

    else
    {
        if(current_node==sloc)
        {
            
            next_node=tree_location[current_node].parent[0];
            printf("Case 0: next node: %d\n", next_node);
            
            if(tree_location[next_node].parent_side == LEFT)
            {
                if(tree_location[current_node].parent_out_domains[0][local_timestep]== DOMAIN_UNUSED && tree_location[next_node].left_in_domains[local_timestep] == DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_0;
                }
            }

            else if(tree_location[next_node].parent_side == RIGHT)
            {
                if(tree_location[current_node].parent_out_domains[0][local_timestep]== DOMAIN_UNUSED && tree_location[next_node].right_in_domains[local_timestep] == DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_0;
                }
            }

            else
            {
                result= DOMAIN_TAKEN;
            }
            
            printf("RESULT: %d local_timestep %d\n", result, local_timestep);

            if(result==DOMAIN_UNUSED_0)
            {
                insert_path(route_array[route_num], current_node, PARENT_OUT_0);
                //step_num++;
                if(tree_location[next_node].parent_side == RIGHT)
                {
                    insert_path(route_array[route_num], next_node, RIGHT_IN)
                }
                else
                {
                    insert_path(route_array[route_num], next_node, LEFT_IN)
                }
                
                //step_num++;
                route_source_sink(sloc,loc,next_node, current_node, src,dst,whichNet, route_num, local_timestep, step_num, route_array); 
            }
            
            else
            {
                printf("RETURNING FALSE\n");
                return FALSE;
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
                printf("AVAILABLE? %d\n", tree_location[next_node].left_in_domains[local_timestep] );
                
                if(tree_location[next_node].parent_side == LEFT)
                {
                    if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].left_in_domains[local_timestep]==DOMAIN_UNUSED)
                    {
                        result= DOMAIN_UNUSED_0;
                    }
                    else if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].left_in_domains[local_timestep]==DOMAIN_UNUSED && result!= DOMAIN_UNUSED_0)
                    {
                        result= DOMAIN_UNUSED_1;
                    }
                    else
                    {
                        result= DOMAIN_TAKEN;
                    }
                }

                else if(tree_location[next_node].parent_side == RIGHT)
                {
                    if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].right_in_domains[local_timestep]==DOMAIN_UNUSED)
                    {
                        result= DOMAIN_UNUSED_0;
                    }
                    else if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].right_in_domains[local_timestep]==DOMAIN_UNUSED && result!= DOMAIN_UNUSED_0)
                    {
                        result= DOMAIN_UNUSED_1;
                    }
                    else
                    {
                        result= DOMAIN_TAKEN;
                    }
                }

                if(result==DOMAIN_UNUSED_0)
                {
                    insert_path(route_array[step_num], current_node, PARENT_OUT_0)

                    if(tree_location[next_node].parent_side == RIGHT)
                    {
                        insert_path(route_array[step_num], next_node, RIGHT_IN)
                    }

                    else
                    {
                        insert_path(route_array[step_num], next_node, LEFT_IN)
                    }

                    route_source_sink(sloc,loc,next_node, current_node, src, dst, whichNet, route_num, local_timestep, step_num, route_array); 
                } 

                else if(result==DOMAIN_UNUSED_1)
                {
                    insert_path(route_array[step_num], current_node, PARENT_OUT_1)

                    if(tree_location[next_node].parent_side == RIGHT)
                    {
                        insert_path(route_array[step_num], next_node, RIGHT_IN)
                    }

                    else
                    {
                        insert_path(route_array[step_num], next_node, LEFT_IN)
                    }

                    route_source_sink(sloc,loc,next_node, current_node, src, dst, whichNet, route_num, local_timestep, step_num, route_array); 
                }

                else
                {
                    return FALSE;
                }
            }

            else if (path_to_leaf_side(tree_location[current_node].left,block[dst].loc)==TRUE)
            {
                
                next_node=tree_location[current_node].left;
                printf("Case 2: next node: %d\n", next_node);
                result= tree_location[next_node].parent_in_domains[0][local_timestep]; 
                printf("result: %d\n", result);
                //result=DOMAIN_UNUSED;

                if(result == DOMAIN_UNUSED)
                {
                    printf("local timestep: %d. step_num: %d, next node %d\n", local_timestep, step_num, next_node);
                    route_array[local_timestep][step_num]=next_node;
                    step_num++;
                    route_source_sink(sloc,loc,next_node, current_node, src,dst,whichNet, route_num, local_timestep, step_num, route_array);
                }

                else
                {
                    return FALSE;
                }
               
            }

            else if (path_to_leaf_side(tree_location[current_node].right,block[dst].loc)==TRUE)
            {
                
                next_node=tree_location[current_node].right;
                printf("Case 3: next node: %d\n", next_node);
                printf("local_timestep=%d, results %d\n", local_timestep, tree_location[next_node].parent_in_domains[0][local_timestep]);
                result = tree_location[next_node].parent_in_domains[0][local_timestep];
                printf("result: %d\n", result);

                if(result==DOMAIN_UNUSED)
                {
                    printf("local timestep: %d. step_num: %d, next node %d\n", local_timestep, step_num, next_node);
                    route_array[local_timestep][step_num]=next_node;
                    step_num++;
                    printf("Filled the array, what happened\n");
                    route_source_sink(sloc,loc,next_node, current_node, src,dst,whichNet, route_num, local_timestep, step_num, route_array);
                }

                else
                {
                    return FALSE;
                }
            }
        }
    }   
}


void insert_route(int whichNet, int local_timestep, int **route_array, int maximum_length)
{
    int i;
    int j;
    for(i=0; i<net[whichNet].num_pins; i++)
    {
        for (j=0; j<maximum_length; j++)
        {
            if (route_array[i][j]!=-1)
            {
                printf("Pin %d, location: %d\n", i, route_array[i][j]);
            }
            
        }
        printf("\n");
        //tree_location[route_array[i][j]];
        /* GOING UP */
        // Left in, parent out 0

        // Left in, parent out 1

        // Right in, parent out 0

        // Right in, parent out 1

        /* GOING DOWN */
        // Left out, parent in 0

        // Left out, parent in 1

        // Right out, parent in 0

        // Right out, parent in 1

        /* GOING ACROSS */
        // Left in 0, right out 0

        // Left in 1, right out 1

        // Right in 0, left out 0

        // Right in 1, left in 1

        // Left in 1, right out 0

        // Left in 0, right out 1

        // Right in 0, left out 1

        // Right in 1, left out 0

    }

}
