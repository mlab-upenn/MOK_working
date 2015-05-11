#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "util.h"
#include "main.h"
#include "globals.h"
#include "tree.h"
#include "asap.h"
#include "domain.h"
#include "part_place.h"
#include "updategains.h"
#include <string.h>
#include "levels.h"

#define DEBUG_PART_PLACE

int initialize_levels(void)
{
    int i;
    int maxLevel=0;
    for(i=0; i<num_blocks; i++)
    {
        block[i].level=level[i];
        if(level[i]>maxLevel)
        {
            maxLevel=level[i];
        }
    }
    return maxLevel;
}


int post_klfm_balance_by_level(int location, int whatLevel)
{
    int i;
    int left_count=0;
    int right_count=0;
    int makespan=find_max_timestep();
    for(i=0; i<num_blocks; i++)
    {
        if(block[i].level==whatLevel && block[i].left==1)
        {
            left_count=left_count+1;
        }
        if(block[i].level==whatLevel && block[i].left==0)
        {
            right_count=right_count+1;
        }
    }
    //printf("Level %d left count: %d right_count %d\n", whatLevel, left_count, right_count);
    int balance=abs(left_count-right_count);
    return balance;

}

int post_klfm_balance_for_all_levels(int location)
{
    int i;
    int max_level=initialize_levels();
    int temp=0;
    int max_unb=0;
    int active_level=-1;

    for(i=0; i<max_level+1; i++)
    {
        temp=post_klfm_balance_by_level(location, i);
        if(temp>max_unb)
            active_level=i;
    }
    return active_level;
}


void initialize_gain_by_level(int location)
{
    int i;
    int j;
    int k;
    int max_level=initialize_levels();
    max_level=max_level+1;


    for(i=0; i<num_nets;i++)
    {
        net[i].F_left_level=(int *)malloc(sizeof(int)*max_level);
        net[i].T_left_level=(int *)malloc(sizeof(int)*max_level);
        net[i].F_right_level=(int *)malloc(sizeof(int)*max_level);
        net[i].T_right_level=(int *)malloc(sizeof(int)*max_level);
        for (j=0; j<max_level;j++)
        {
            net[i].T_left_level[j]=0; 
            net[i].F_left_level[j]=0;
            net[i].T_right_level[j]=0;
            net[i].F_right_level[j]=0;
        }
    }

    for(i=0; i<num_blocks;i++)
    {
        block[i].level_gain=(int *)malloc(sizeof(int)*max_level);
        for (j=0; j<max_level;j++)
        {
           block[i].level_gain[j]=0;
        }
    }

    for(k=0; k<max_level; k++)
    {
        for(i=0; i<num_nets; i++)
        {
            for(j=0; j<net[i].num_pins; j++)
            {
                int current_block=net[i].pins[j];
                #ifdef DEBUG_PART_PLACE
                printf("Current block: %s\n", block[current_block].name);
                printf("Left evals %d\n",block[current_block].left);
                #endif

                if(block[current_block].left==1 && block[current_block].level==k)
                {
                    #ifdef DEBUG_PART_PLACE
                    printf("Current block: %s caused F_left increase \n", block[current_block].name);
                    printf("sub left evals %d\n",block[current_block].left);
                    #endif        
                    net[i].F_left_level[k]=net[i].F_left_level[k]+1;
                    net[i].T_right_level[k]=net[i].T_right_level[k]+1;
                }

                else if(block[current_block].left==0 && block[current_block].level==k)
                { 
                    #ifdef DEBUG_PART_PLACE
                    printf("Current block: %s caused T_left increase \n", block[current_block].name);
                    printf("sub left evals %d\n",block[current_block].left);
                    #endif
                    net[i].T_left_level[k]=net[i].T_left_level[k]+1;
                    net[i].F_right_level[k]=net[i].F_right_level[k]+1;
                }
            }
        } 
    }

    for(k=0; k<max_level; k++)
    {
    // Loop through all of the blocks which are present in the tree structure's left child
        for (i=0;i<tree_location[tree_location[location].left].used_slots;i++)
        { 
            int numberOfNets=0;
            int start = 0;
            int current_block=tree_location[tree_location[location].left].slots[i];

            #ifdef DEBUG_PART_PLACE
            printf("Current Block: %s\n", block[current_block].name);
            #endif

            // Figure out how many nets are connected to the block
            if(block[current_block].type==LATCH || block[current_block].type==LUT_AND_LATCH)
            {
              numberOfNets=block[current_block].num_nets-1;
              start=0;
            }

            else
            {
              numberOfNets=block[current_block].num_nets;
              start=0;
            }

        // For all the nets
        for (j=start; j<numberOfNets; j++)
          {
            #ifdef DEBUG_PART_PLACE
            printf (" j %d, F_left %d number of nets %d\n", j,net[block[current_block].nets[j]].F_left_level[k], numberOfNets);
            printf("T_left %d\n", net[block[current_block].nets[j]].T_left_level[k]);
            #endif

            if(net[block[current_block].nets[j]].F_left_level[k]==1)
            {
                 // printf("in here\n");
                block[current_block].level_gain[k]=block[current_block].level_gain[k]+1;
                #ifdef DEBUG_PART_PLACE
                //printf("gain increased %d\n",gain[current_block]);
                #endif
            }
            else if(net[block[current_block].nets[j]].T_left_level[k]==0)
            {
                block[current_block].level_gain[k]=block[current_block].level_gain[k]-1;

                #ifdef DEBUG_PART_PLACE
                //printf("gain decreased %d\n", gain[current_block]);
                #endif
            }

        }        

    }
    }


    for(k=0; k<max_level; k++)
    {

        //printf("used slots %d\n", tree_location[tree_location[location].right].used_slots);
        // Loop through all of the blocks which are present in the tree structure's left child
        for (i=0;i<tree_location[tree_location[location].right].used_slots;i++)
        {   
            int numberOfNets=0;
            int start = 0;
            int current_block=tree_location[tree_location[location].right].slots[i];

            #ifdef DEBUG_PART_PLACE
            printf("Current Block: %s\n", block[current_block].name);
            #endif

            // Figure out how many nets are connected to the block
            if(block[current_block].type==LATCH || block[current_block].type==LUT_AND_LATCH)
            {
              numberOfNets=block[current_block].num_nets-1;
              start=0;
            }

            else
            {
              numberOfNets=block[current_block].num_nets;
              start=0;
            }

        // For all the nets
        for (j=start; j<numberOfNets; j++)
          {
            #ifdef DEBUG_PART_PLACE
            printf (" j %d, F_right %d number of nets %d\n", j,net[block[current_block].nets[j]].F_right_level[k], numberOfNets);
            printf("T_right %d\n", net[block[current_block].nets[j]].T_right_level[k]);
            #endif

            if(net[block[current_block].nets[j]].F_right_level[k]==1)
            {
                block[current_block].level_gain[k]=block[current_block].level_gain[k]+1;
                #ifdef DEBUG_PART_PLACE
                printf("gain increased %d\n",block[current_block].level_gain[k]);
                #endif
            }
            else if(net[block[current_block].nets[j]].T_right_level[k]==0)
            {
                block[current_block].level_gain[k]=block[current_block].level_gain[k]-1;
                #ifdef DEBUG_PART_PLACE
                printf("gain decreased %d\n", block[current_block].level_gain[k]);
                #endif
            }

        }        

    }
    
    }

    for (i=0; i<num_blocks; i++)
    {
        int temp=0;
        for(j=0; j<max_level; j++)
        {   
            temp=temp+block[i].level_gain[j];
            //printf("Block %s current level gain %d level %d temp %d\n", block[i].name, block[i].level_gain[j], j, temp);
        }
        block[i].cost_func=temp+block[i].gain;
        //printf("cost fucntion + gain = %d\n", block[i].cost_func);
        block[i].cost_func=floor(block[i].cost_func/(max_level+2));
        //printf("COST FUNCTION POST INIT IS %d\n", block[i].cost_func);
    }

}





