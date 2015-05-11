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
#define PARENT_OUT_0 -2
#define PARENT_OUT_1 -3
#define PARENT_IN_0 -4
#define PARENT_IN_1 -5
#define LEFT_IN -6
#define RIGHT_IN -7
#define LEFT_OUT -8
#define RIGHT_OUT -9
#define DOMAIN_UNUSED_0 -10
#define DOMAIN_UNUSED_1 -11
#define DOMAIN_USED -12

#define DEBUG_DROUTE

void detail_route(boolean *is_clock, boolean global_clock, int maxwaves, boolean verbose)
{
    int i;

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
    alap_delay();
    #ifdef DEBUG_DROUTE
    printf("Max timestep: %d\n",alap_time_max);
    #endif
}

void route_all_nets(boolean *is_clock, boolean global_clock, boolean verbose, int *alap_sort)
{
    int i;
    for(i=num_blocks-1; i>-1; i--)
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
    static boolean route_outcome= FALSE;
    // Store the current node
    int current_node;
    // src is the block
    int src=net[whichNet].pins[0];
    printf("SOURCE= %s\n", block[src].name);
    // sloc is the PE
    int sloc=block[src].loc;
    // reset local timestep
    int local_timestep=0;
    // the minimum step intially can't be worse than alap_time_max
    int max_step=UNASSIGNED_TIMESTEP;
    // compute the height of the tree
    int height=get_nodes();
    // the maximum path length can't be more than 2x height
    int max_path=2*ilog2(height);

    // for source nodes that are not clocks compute min step
    if(block[src].loc!=GLOBAL_CLOCK_LOC)
    {
        int start=1;
        if (block[src].type==OUTPAD)
            start=0;

        for (j=start;j<block[src].num_nets;j++)
        {
            if (block[src].nets[j]!=UNDEFINED)
            {
                int tempNet=block[src].nets[j];
                int pred=net[tempNet].pins[0];
                int pred_timestep=timestep(pred);
                printf("timestep %d, block: %s\n", pred_timestep, block[pred].name);
                if(pred_timestep>max_step)
                {
                    max_step=pred_timestep;
                }
            }
            
        }
    }
    else
    {
        printf("SOURCE: %s is a clock\n", block[src].name);
    }

    // if the minimum step is unassigned we are dealing with an input block
    if(max_step==UNASSIGNED_TIMESTEP)
    {
        local_timestep=0;
    }

    // otherwise assign the min step value
    else
    {
        local_timestep=max_step+1;
    }

    printf("SOURCE: %s, initial timestep: %d\n", block[src].name, local_timestep);

    // create a pointer for the route array
    ListPtr *route_array;

    // again, if the block is not a clock we need to allocate space for its path
    // initialize this path with -1
    if(block[src].loc!=GLOBAL_CLOCK_LOC)
    {
        route_array=(ListPtr*)malloc((net[whichNet].num_pins+1)*sizeof(ListPtr));
        
        for(i=0; i<net[whichNet].num_pins+1; i++)
        {
            route_array[i]=(ListPtr)malloc(sizeof(ListNode));
            insert_path(&route_array[i],-1,-1,-1);
        }
    }

    // now that space has been allocated for a path
    // if the src is not a clock
    // attempt to compute the detailed route
    // if fail, increase local timestep
    if(block[src].loc!=GLOBAL_CLOCK_LOC)
    {
        j=1;
        while (j<net[whichNet].num_pins)
        {
          int dst=net[whichNet].pins[j];
          int loc=block[dst].loc;
          current_node=src;

          #ifdef DEBUG_DROUTE
          printf("About to route %d to %d\n",src,dst);
          #endif

          route_outcome=route_source_sink(sloc,loc,sloc, sloc, src, dst, whichNet, j, local_timestep, 0, route_array);

          if (route_outcome==FALSE)
          {
            #ifdef DEBUG_DROUTE
            printf("Route failed\n");
            #endif
            // Go back to start of the for loop and reroute
            cleanup_reroute(route_array, whichNet);
            #ifdef DEBUG_DROUTE
            printf("Finished cleanup\n");
            #endif

            j=1;
            local_timestep++;

            #ifdef DEBUG_DROUTE
            printf("Incremented the local timestep to %d\n", local_timestep); 
            #endif
          }
          else 
          {
            j++;
          }
        }
        update_timestep(src, local_timestep); 
        insert_route(whichNet, route_array);
    }
}


boolean route_source_sink(int sloc, int loc, int current_node, int previous_node, int src, int dst, 
                          int whichNet, int route_num, int local_timestep, int step_num, ListPtr *route_array)
{
    int next_node;
    int result;
    boolean next_route= FALSE;

    #ifdef DEBUG_DROUTE
    printf("Global Route %s(%d) %d->%d:\n",net[whichNet].name,whichNet,sloc,loc);
    printf("Current node: %d Source: %d\n", current_node, src);
    #endif

    // ALL DONE
    if(current_node==loc)
    {
        #ifdef DEBUG_DROUTE
        printf("Reached destination\n\n");
        #endif
        if(previous_node!=current_node)
        {
            if (tree_location[current_node].parent_in_domains[0][local_timestep]== DOMAIN_UNUSED)
            {
                insert_path(&route_array[route_num], current_node, PARENT_IN_0, local_timestep);
                return TRUE;

            }
            else if (tree_location[current_node].parent_in_domains[1][local_timestep]== DOMAIN_UNUSED)
            {
                insert_path(&route_array[route_num], current_node, PARENT_IN_1, local_timestep);

                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        else
        {
            return TRUE;
        }
        
    }

    else
    {
        // CASE 0
        if(current_node==sloc)
        {

            next_node=tree_location[current_node].parent[0];
            #ifdef DEBUG_DROUTE
            printf("Case 0: next node: %d\n", next_node);
            #endif
            if(tree_location[current_node].parent_side == LEFT)
            {

                if(tree_location[current_node].parent_out_domains[0][local_timestep]== DOMAIN_UNUSED && tree_location[next_node].left_in_domains[local_timestep] == DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_0;
                    #ifdef DEBUG_DROUTE
                    printf("Case 0: Left Parent 0\n");
                    #endif
                }
                else
                {
                    result= DOMAIN_USED;
                    #ifdef DEBUG_DROUTE
                    printf("Case 0: Left Parent 0 FAILURE\n");
                    #endif
                }
            }

            else if(tree_location[current_node].parent_side == RIGHT)
            {
                if(tree_location[current_node].parent_out_domains[0][local_timestep]== DOMAIN_UNUSED && tree_location[next_node].right_in_domains[local_timestep] == DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_0;
                    #ifdef DEBUG_DROUTE
                    printf("Case 0: Right Parent 0\n");
                    #endif
                }
                else
                {
                    result= DOMAIN_USED;
                    #ifdef DEBUG_DROUTE
                    printf("Case 0: Right Parent 0 FAILURE\n");
                    #endif
                }
            }

            else
            {
                result= DOMAIN_USED;
                #ifdef DEBUG_DROUTE
                printf("Case 0: Complete failure?\n");
                #endif
            }

            if(result==DOMAIN_UNUSED_0)
            {
                insert_path(&route_array[route_num], current_node, PARENT_OUT_0, local_timestep);

                if(tree_location[current_node].parent_side == RIGHT)
                {
                    insert_path(&route_array[route_num], next_node, RIGHT_IN, local_timestep);
                }

                else
                {
                    insert_path(&route_array[route_num], next_node, LEFT_IN, local_timestep);
                }

                next_route=route_source_sink(sloc,loc,next_node, current_node, src,dst,whichNet, route_num, local_timestep, step_num, route_array); 
            }
            
            else
            {
                #ifdef DEBUG_DROUTE
                printf("Case 0: RETURNING FALSE\n");
                #endif
                return FALSE;
            }
        }

        // CASE 1
        else if ((path_to_leaf_side(current_node,sloc)==TRUE) && (path_to_leaf_side(current_node,block[dst].loc)==FALSE))
        {

            next_node=tree_location[current_node].parent[0];
            #ifdef DEBUG_DROUTE
            printf("Case 1: next node: %d\n", next_node);
            #endif

            if(tree_location[next_node].parent_side == LEFT)
            {
                if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].left_in_domains[local_timestep]==DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_0;
                    #ifdef DEBUG_DROUTE
                    printf("Case 1: Left Parent 0\n");
                    #endif
                }
                else if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].left_in_domains[local_timestep]==DOMAIN_UNUSED && result!= DOMAIN_UNUSED_0)
                {
                    result= DOMAIN_UNUSED_1;
                    #ifdef DEBUG_DROUTE
                    printf("Case 1: Left Parent 1\n");
                    #endif
                }
                else
                {
                    result= DOMAIN_USED;
                    #ifdef DEBUG_DROUTE
                    printf("Case 1: Left Parent FAILURE\n");
                    #endif
                }
            }

            else if(tree_location[next_node].parent_side == RIGHT)
            {
                if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].right_in_domains[local_timestep]==DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_0;
                    #ifdef DEBUG_DROUTE
                    printf("Case 1: Right Parent 0\n");
                    #endif
                }
                else if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].right_in_domains[local_timestep]==DOMAIN_UNUSED && result!= DOMAIN_UNUSED_0)
                {
                    result= DOMAIN_UNUSED_1;
                    #ifdef DEBUG_DROUTE
                    printf("Case 1: Right Parent 1\n");
                    #endif
                }
                else
                {
                    result= DOMAIN_USED;
                    #ifdef DEBUG_DROUTE
                    printf("Case 1: Right Parent FAILURE\n");
                    #endif
                }
            }

            if(result==DOMAIN_UNUSED_0)
            {
                insert_path(&route_array[route_num], current_node, PARENT_OUT_0, local_timestep);

                if(tree_location[current_node].parent_side == RIGHT)
                {
                    insert_path(&route_array[route_num], next_node, RIGHT_IN, local_timestep);
                }

                else
                {
                    insert_path(&route_array[route_num], next_node, LEFT_IN, local_timestep);
                }

                next_route=route_source_sink(sloc,loc,next_node, current_node, src, dst, whichNet, route_num, local_timestep, step_num, route_array); 
            } 

            else if(result==DOMAIN_UNUSED_1)
            {
                insert_path(&route_array[route_num], current_node, PARENT_OUT_1, local_timestep);

                if(tree_location[current_node].parent_side == RIGHT)
                {
                    insert_path(&route_array[route_num], next_node, RIGHT_IN, local_timestep);
                }

                else
                {
                    insert_path(&route_array[route_num], next_node, LEFT_IN, local_timestep);
                }

                next_route=route_source_sink(sloc,loc,next_node, current_node, src, dst, whichNet, route_num, local_timestep, step_num, route_array); 
            }

            else
            {
                #ifdef DEBUG_DROUTE
                printf("Case 1: RETURNING FALSE\n");
                #endif
                return FALSE;
            }
        }

        // CASE 2
        else if (path_to_leaf_side(tree_location[current_node].left,block[dst].loc)==TRUE)
        {
            next_node=tree_location[current_node].left;
            #ifdef DEBUG_DROUTE
            printf("Case 2: next node: %d\n", next_node);
            #endif

            if(tree_location[next_node].parent_in_domains[0][local_timestep]== DOMAIN_UNUSED && tree_location[current_node].left_out_domains[local_timestep]==DOMAIN_UNUSED)
            {
                result= DOMAIN_UNUSED_0;
                #ifdef DEBUG_DROUTE
                printf("Case 2: Parent 0 in, Left out\n");
                #endif
            }
            else if(tree_location[next_node].parents>1 && result != DOMAIN_UNUSED_0)
            {
                if(tree_location[next_node].parent_in_domains[1][local_timestep]== DOMAIN_UNUSED && tree_location[current_node].left_out_domains[local_timestep]==DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_1;
                    #ifdef DEBUG_DROUTE
                    printf("Case 2: Parent 1 in, Left out\n");
                    #endif
                }
                
            }
            else
            {
                result= DOMAIN_USED;
                #ifdef DEBUG_DROUTE
                printf("Case 2: Left out FAILURE\n");
                #endif
            }


            if(result == DOMAIN_UNUSED_0)
            {
                #ifdef DEBUG_DROUTE
                printf("local timestep: %d. step_num: %d, next node %d\n", local_timestep, step_num, next_node);
                #endif

                insert_path(&route_array[route_num], current_node, LEFT_OUT, local_timestep);
                insert_path(&route_array[route_num], next_node, PARENT_IN_0, local_timestep);

                next_route=route_source_sink(sloc,loc,next_node, current_node, src,dst,whichNet, route_num, local_timestep, step_num, route_array);
            }
            else if( result == DOMAIN_UNUSED_1)
            {
                insert_path(&route_array[route_num], current_node, LEFT_OUT, local_timestep);
                insert_path(&route_array[route_num], next_node, PARENT_IN_1, local_timestep);

                next_route=route_source_sink(sloc,loc,next_node, current_node, src,dst,whichNet, route_num, local_timestep, step_num, route_array);
            }

            else
            {
                #ifdef DEBUG_DROUTE
                printf("Case 2: RETURNING FALSE\n");
                #endif
                return FALSE;
            }

        }

        // CASE 3
        else if (path_to_leaf_side(tree_location[current_node].right,block[dst].loc)==TRUE)
        {

            next_node=tree_location[current_node].right;
            #ifdef DEBUG_DROUTE
            printf("Case 3: next node: %d\n", next_node);
            #endif

            if(tree_location[next_node].parent_in_domains[0][local_timestep]== DOMAIN_UNUSED && tree_location[current_node].right_out_domains[local_timestep]==DOMAIN_UNUSED)
            {
                result= DOMAIN_UNUSED_0;
                #ifdef DEBUG_DROUTE
                printf("Case 3: Parent 0 in, Right out\n");
                #endif
            }
            else if(tree_location[next_node].parents>1 && result!=DOMAIN_UNUSED_0)
            {
                if(tree_location[next_node].parent_in_domains[1][local_timestep]== DOMAIN_UNUSED && tree_location[current_node].right_out_domains[local_timestep]==DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_1;
                    #ifdef DEBUG_DROUTE
                    printf("Case 3: Parent 0 in, Right out\n");
                    #endif
                }
            }
            
            else
            {
                result= DOMAIN_USED;
                #ifdef DEBUG_DROUTE
                printf("Case 2: Right out FAILURE\n");
                #endif
            }


            if(result == DOMAIN_UNUSED_0)
            {
                #ifdef DEBUG_DROUTE
                printf("local timestep: %d. step_num: %d, next node %d\n", local_timestep, step_num, next_node);
                #endif

                insert_path(&route_array[route_num], current_node, RIGHT_OUT, local_timestep);
                insert_path(&route_array[route_num], next_node, PARENT_IN_0, local_timestep);

                next_route=route_source_sink(sloc,loc,next_node, current_node, src,dst,whichNet, route_num, local_timestep, step_num, route_array);
            }
            else if( result == DOMAIN_UNUSED_1)
            {
                insert_path(&route_array[route_num], current_node, RIGHT_OUT, local_timestep);
                insert_path(&route_array[route_num], next_node, PARENT_IN_1, local_timestep);

                next_route=route_source_sink(sloc,loc,next_node, current_node, src,dst,whichNet, route_num, local_timestep, step_num, route_array);
            }

            else
            {
                #ifdef DEBUG_DROUTE
                printf("Case 3: RETURNING FALSE\n");
                #endif
                return FALSE;
            }
        }
        if(next_route==FALSE)
        {
            return FALSE;
        }
    }      
}

void cleanup_reroute(ListPtr *route_array, int whichNet)
{
    int i;
    ListPtr head;
    ListPtr temp;
    int location;

    #ifdef DEBUG_DROUTE
    printf("Cleanup and reset route array\n");
    #endif

    for(i=1; i<net[whichNet].num_pins; i++)
    {
        // Get pointer to head of list
        head=route_array[i];
        // Initialize values
        location = head->location;
        // While we still have items left in the list fill the domains
        while(location != -1)
        {
            temp = head->nextPtr;
            head=temp;
            if(temp!=NULL)
            free(temp);
            location = head->location;
            #ifdef DEBUG_DROUTE
            printf("Location: %d\n", location);
            #endif
            
        }
        #ifdef DEBUG_DROUTE
        printf("EXIT\n");
        #endif
        head->location=-1;
        route_array[i]=head;
        #ifdef DEBUG_DROUTE
        printf("Reset route_array[i] to head\n");
        #endif
    }

    route_array=malloc(net[whichNet].num_pins*sizeof(ListPtr));
        
        for(i=0; i<net[whichNet].num_pins; i++)
        {
            route_array[i]= malloc(sizeof(ListNode));
            insert_path(&route_array[i],-1,-1, -1);
        }
}

void insert_route(int whichNet, ListPtr *route_array)
{
   ListPtr head;
   ListPtr temp;
   int i;
   int location;
   int route_type;
   int local_timestep;

   for(i=1; i<net[whichNet].num_pins; i++)
   {
        // Get pointer to head of list
        head=route_array[i];
        // Initialize values
        location = head->location;
        route_type = head-> detail_loc;
        local_timestep = head -> local_timestep;

        // While we still have items left in the list fill the domains
        while(location != -1)
        {
            #ifdef DEBUG_DROUTE
            printf("Placed: location: %d, route type: %d, local timestep: %d\n", location, route_type, local_timestep);
            #endif
            place_in_domain(location, route_type, local_timestep, whichNet);
            temp = head->nextPtr;
            head=temp;
            //if(temp!=NULL);
            //free(temp);
            location = head->location;
            #ifdef DEBUG_DROUTE
            printf("location: %d\n", location);
            #endif
            route_type = head-> detail_loc;
            local_timestep = head -> local_timestep;
            
        }
        #ifdef DEBUG_DROUTE
        printf("EXIT\n");
        printf("\n");
        #endif
   }

   return;
}

void place_in_domain(int location, int route_type, int local_timestep, int whichNet)
{
    //printf("In place in domain\n");
    if(route_type == PARENT_OUT_0)
    {
        tree_location[location].parent_out_domains[0][local_timestep]=whichNet;
    }
    else if (route_type == PARENT_OUT_1)
    {
        tree_location[location].parent_out_domains[1][local_timestep]=whichNet;
    }
    else if (route_type == PARENT_IN_0)
    {
        tree_location[location].parent_in_domains[0][local_timestep]=whichNet;
    }
    else if (route_type == PARENT_IN_1)
    {
        tree_location[location].parent_in_domains[1][local_timestep]=whichNet;
    }
    else if ( route_type == LEFT_IN )
    {
        tree_location[location].left_in_domains[local_timestep]=whichNet;
    }
    else if( route_type == RIGHT_IN)
    {
        tree_location[location].right_in_domains[local_timestep]=whichNet;
    }
    else if( route_type == LEFT_OUT)
    {
        tree_location[location].left_out_domains[local_timestep]=whichNet;
    }
    else if (route_type == RIGHT_OUT)
    {
        tree_location[location].right_out_domains[local_timestep]=whichNet;
    }
    else
    {
        #ifdef DEBUG_DROUTE
        printf("Nothing to place, something went wrong\n");
        #endif
        return;
    }

}
