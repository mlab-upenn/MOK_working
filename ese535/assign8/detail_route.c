#include <stdio.h>
#include <string.h>
#include "util.h"
#include "main.h"
#include "tree.h"
#include "globals.h"
#include "detail_route.h"
#include "list.h"
#include "domain.h"
#include "asap.h"

//#define INITIAL_CALL -1
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

//#define DEBUG_DROUTE

void detail_route(boolean *is_clock, boolean global_clock, int maxwaves, boolean verbose)
{
    int i;
    //process_block_list();

    // Sort blocks
    //sort_blocks(is_clock,global_clock,FALSE);

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
    int *asap_sort=(int *)my_malloc(sizeof(int)*(num_blocks));
    my_heapsort (asap_sort, level, num_blocks); 
    for (i=0; i<num_blocks; i++)
    {
        printf("%s level: %d\n", block[asap_sort[i]].name, level[asap_sort[i]]);

    }
    //printf("14: %s", block[14].name);
    for(i=0; i<num_blocks; i++)
    {
        int whichNet=block[asap_sort[i]].nets[0];
        //printf("type=%d\n",block[alap_sort[i]].type);
        if(block[asap_sort[i]].type==3 || block[asap_sort[i]].type==1 )
        {
            //printf("ROUTING LATCH\n");
           route_net(whichNet,is_clock,global_clock,FALSE);  
        }
        
    }
    for(i=num_blocks-1; i>-1; i--)
    //for(i=0; i<num_blocks; i++)
    {
        //int whichNet=block[alap_sort[i]].nets[0];
        
        int whichNet=block[asap_sort[i]].nets[0];
        if(block[asap_sort[i]].type!=3 && block[asap_sort[i]].type!=1 )
        {
           route_net(whichNet,is_clock,global_clock,FALSE);  
        }
               
    }
}

void route_net(int whichNet, boolean *is_clock, boolean global_clock, boolean verbose)
{
    // Loop counters
    int j;
    int i;
    static boolean route_outcome;
    // Store the current node
    int current_node;
    // src is the block
    int src=net[whichNet].pins[0];
    //printf("SOURCE= %s\n", block[src].name);
    // sloc is the PE
    int sloc=block[src].loc;
    // reset local timestep
    static int local_timestep=0;
    // the minimum step intially can't be worse than alap_time_max
    int max_step=-1;

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
            if(pred_timestep<0)
                {
                    pred_timestep=0;
                }
            //if(src==244)
            //{
                //printf("src=%s pred= %s timestep= %d, pred type: %d\n", block[src].name, block[pred].name, pred_timestep, block[pred].type);
                //printf("\n");
            //}

            #ifdef DEBUG_DROUTE
            printf("timestep %d, block: %s\n", pred_timestep, block[pred].name);
            #endif

            if(pred_timestep>max_step)
            {
                max_step=pred_timestep;
            }
        }   
    }

    local_timestep=max_step+1;
    if(local_timestep<0)
    {
        local_timestep=0;
    }


    //#ifdef DEBUG_DROUTE
    //printf("SOURCE: %s, initial timestep: %d\n", block[src].name, local_timestep);
    //#endif

    // create a pointer for the route array
    ListPtr *route_array=malloc((net[whichNet].num_pins+1)*sizeof(ListPtr));

    // again, if the block is not a clock we need to allocate space for its path
    // initialize this path with -1
    if(block[src].loc!=GLOBAL_CLOCK_LOC)
    {

        int start=0;

        for(i=start; i<net[whichNet].num_pins+1; i++)
        {
            route_array[i]=NULL;
        }

        for(i=start; i<net[whichNet].num_pins+1; i++)
        {
            route_array[i]=(ListPtr)malloc((net[whichNet].num_pins+1)*sizeof(ListNode));
            route_array[i]->location=-1;
            route_array[i]->detail_loc=-1;
            route_array[i]->local_timestep=-1;
            route_array[i]->nextPtr=NULL;
        }
    }

    // now that space has been allocated for a path
    // if the src is not a clock
    // attempt to compute the detailed route
    // if fail, increase local timestep
    if(block[src].loc!=GLOBAL_CLOCK_LOC)
    {
        int start=1;
        if (block[src].type==OUTPAD)
            start=0;
        j=start;

        while (j<net[whichNet].num_pins)
        {
          int dst=net[whichNet].pins[j];
          int loc=block[dst].loc;
          current_node=src;

          #ifdef DEBUG_DROUTE
          printf("About to route %d to %d\n",src,dst);
          #endif

          if(src!=dst && sloc!=loc)
          {
            route_outcome=route_source_sink(sloc,loc,sloc, sloc, src, dst, whichNet, j, local_timestep, 0, route_array);
          }
          else
          {
            route_outcome=TRUE;
          }
          

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

            j=start;
            local_timestep++;
            if(local_timestep>100000)
            {
                //printf("CRAP\n");
            }

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
    boolean next_route;

    #ifdef DEBUG_DROUTE
    printf("Global Route %s(%d) %d->%d:\n",net[whichNet].name,whichNet,sloc,loc);
    
    printf("Current node: %d Source: %d\n", current_node, src);
    #endif

    // ALL DONE
    if(current_node==loc)
    {
        if(sloc!=loc)
        {
            if(tree_location[current_node].parent_in_domains[0][local_timestep]!=DOMAIN_USED)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        
        else
        {
        #ifdef DEBUG_DROUTE
        printf("Reached destination\n\n");
        printf("returning true\n");
        #endif

        return TRUE;
        }
        
    }

    else
    {
        // CASE 0
        /*if(current_node==sloc)
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
                    printf("Case 0: Right Parent 0\n");
                    #endif
                }
                else
                {
                    result= DOMAIN_USED;

                    #ifdef DEBUG_DROUTE
                    printf("Case 0: Right Parent 0 FAILURE\n");
                    printf("why fail? %d, %d\n",tree_location[current_node].parent_out_domains[0][local_timestep],tree_location[next_node].left_in_domains[local_timestep]);
                    #endif
                }
            }

            else if(tree_location[current_node].parent_side == RIGHT)
            {
                if(tree_location[current_node].parent_out_domains[0][local_timestep]== DOMAIN_UNUSED && tree_location[next_node].right_in_domains[local_timestep] == DOMAIN_UNUSED)
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

                if(tree_location[current_node].parent_side == LEFT)
                {
                    insert_path(&route_array[route_num], next_node, LEFT_IN, local_timestep);   
                }

                else
                {
                    insert_path(&route_array[route_num], next_node, RIGHT_IN, local_timestep);
                }

                next_route=route_source_sink(sloc,loc,next_node, current_node, src, dst, whichNet, route_num, local_timestep, step_num, route_array); 
            }
            
            else
            {
                #ifdef DEBUG_DROUTE
                printf("Case 0: RETURNING FALSE\n");
                #endif

                return FALSE;
            }
        }*/

        // CASE 1
        if ((path_to_leaf_side(current_node,block[dst].loc)==FALSE))
        {

            next_node=tree_location[current_node].parent[0];

            #ifdef DEBUG_DROUTE
            printf("Case 1: next node: %d\n", next_node);
            #endif

            if(tree_location[current_node].parent_side == LEFT)
            {
                if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].left_in_domains[local_timestep]==DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_0;

                    #ifdef DEBUG_DROUTE
                    printf("Case 1: Right Parent 0\n");
                    #endif
                }
                else if(tree_location[current_node].parents >1)
                {
                    if(tree_location[current_node].parent_out_domains[1][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].left_in_domains[local_timestep]==DOMAIN_UNUSED)
                    {
                        result= DOMAIN_UNUSED_1;

                        #ifdef DEBUG_DROUTE
                        printf("Case 1: Right Parent 1\n");
                        #endif
                    }
                    else
                    {
                        result=DOMAIN_USED;
                    }
                    
                }
                else
                {
                    result= DOMAIN_USED;

                    #ifdef DEBUG_DROUTE
                    printf("Case 1: Right Parent FAILURE\n");
                    #endif
                }
            }

            else if(tree_location[current_node].parent_side == RIGHT)
            {
                //printf("parent out 0: %d,right in: %d\n",tree_location[current_node].parent_out_domains[0][local_timestep],tree_location[next_node].right_in_domains[local_timestep]);
                if(tree_location[current_node].parent_out_domains[0][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].right_in_domains[local_timestep]==DOMAIN_UNUSED)
                {
                    result= DOMAIN_UNUSED_0;

                    #ifdef DEBUG_DROUTE
                    printf("Case 1: Left Parent 0\n");
                    #endif
                }

                else if(tree_location[current_node].parents >1)
                {
                    if(tree_location[current_node].parent_out_domains[1][local_timestep]==DOMAIN_UNUSED && tree_location[next_node].right_in_domains[local_timestep]==DOMAIN_UNUSED)
                    {
                        result= DOMAIN_UNUSED_1;

                        #ifdef DEBUG_DROUTE
                        printf("Case 1: Right Parent 1\n");
                        #endif
                    }
                    else
                    {
                        result=DOMAIN_USED;
                    }
                    
                }
                else
                {
                    result= DOMAIN_USED;

                    #ifdef DEBUG_DROUTE
                    printf("Case 1: FAILURE\n");
                    #endif
                }
            }

            if(result==DOMAIN_UNUSED_0)
            {
                insert_path(&route_array[route_num], current_node, PARENT_OUT_0, local_timestep);

                if(tree_location[current_node].parent_side == LEFT)
                {
                    insert_path(&route_array[route_num], next_node, LEFT_IN, local_timestep);
                }

                else
                {
                    insert_path(&route_array[route_num], next_node, RIGHT_IN, local_timestep);
                }

                next_route=route_source_sink(sloc,loc,next_node, current_node, src, dst, whichNet, route_num, local_timestep, step_num, route_array); 
            } 

            else if(result==DOMAIN_UNUSED_1)
            {
                insert_path(&route_array[route_num], current_node, PARENT_OUT_1, local_timestep);

                if(tree_location[current_node].parent_side == LEFT)
                {
                    insert_path(&route_array[route_num], next_node, LEFT_IN, local_timestep);
                }

                else
                {
                    insert_path(&route_array[route_num], next_node, RIGHT_IN, local_timestep);
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
    }  

    if(next_route!=TRUE)
    {
        return FALSE;
    } 
    
    else
    {
        return TRUE;
    } 

}

void cleanup_reroute(ListPtr *route_array, int whichNet)
{
    int i;
    ListPtr current;
    ListPtr next;

    #ifdef DEBUG_DROUTE
    printf("Cleanup and reset route array\n");
    #endif

    int src=net[whichNet].pins[0];
    int start=1;
        if (block[src].type==OUTPAD)
            start=0;

    for(i=start; i<net[whichNet].num_pins; i++)
    {
        next = NULL;
        current = route_array[i];
        while (current != NULL) 
        {
            next = current->nextPtr;
            free(current);
            current = next;
        }
        route_array[i]=NULL;    
    }
}

void insert_route(int whichNet, ListPtr *route_array)
{
    int i;
    int location;
    int route_type;
    int local_timestep;
    ListPtr next;
    ListPtr current;
    int src=net[whichNet].pins[0];
    int start=1;
    if (block[src].type==OUTPAD)
        start=0;

    for(i=start; i<net[whichNet].num_pins; i++)
    {
        next= NULL;
        current=route_array[i];
        while (current != NULL) 
        {
            next = current->nextPtr;
            location=current->location;
            route_type=current->detail_loc;
            local_timestep=current->local_timestep;
            //printf("Placed: location: %d, route type: %d, local timestep: %d\n", location, route_type, local_timestep);
            place_in_domain(location, route_type, local_timestep, whichNet);
            free(current);
            current = next;
        }

        //printf("EXIT\n");
        //printf("\n");

        route_array[i]=NULL;
    }

    return;
}

void place_in_domain(int location, int route_type, int local_timestep, int whichNet)
{
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
