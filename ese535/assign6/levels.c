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

//#define DEBUG_PART_PLACE

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


void post_klfm_balance_by_level(int location, int whatLevel)
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
    printf("Level %d left count: %d right_count %d\n", whatLevel, left_count, right_count);

}

void post_klfm_balance_for_all_levels(int location)
{
    int i;
    int max_level=initialize_levels();

    for(i=0; i<max_level+1; i++)
    {
        post_klfm_balance_by_level(location, i);

    }
}


void initialize_gain_by_level(void)
{
    int i;
    int j;
    int k;
    int max_level=initialize_levels();

    int *T_left_level=(int *)malloc(sizeof(int)*(max_level+1));
    int *F_left_level=(int *)malloc(sizeof(int)*(max_level+1));
    int *T_right_level=(int *)malloc(sizeof(int)*(max_level+1));
    int *F_right_level=(int *)malloc(sizeof(int)*(max_level+1));

    for (i=0; i<max_level+1;i++)
    {
        T_left_level[i]=0; 
        F_left_level[i]=0;
        T_right_level[i]=0;
        F_right_level[i]=0;
    }

    for(i=0; i<num_nets;i++)
    {
        net[i].F_left_level=F_left_level;
        net[i].T_left_level=T_left_level;
        net[i].F_right_level=F_right_level;
        net[i].T_right_level=T_right_level;
    }

    for(k=0; k<max_level+1; k++)
    {
        for(i=0; i<num_nets; i++)
        {
            for(j=0; j<net[i].num_pins; j++)
            {
                int current_block=net[i].pins[j];
                #ifdef DEBUG_PART_PLACE
                printf("Current block: %s\n", block[current_block].name);
                printf("sub left evals %d\n",block[current_block].left);
                #endif

                if(block[current_block].left==1 && block[current_block].level==k)
                {
                    #ifdef DEBUG_PART_PLACE
                    printf("Current block: %s caused F_left increase \n", block[current_block].name);
                    printf("sub left evals %d\n",block[current_block].left);
                    #endif        
                    net[i].F_left_level[k]=net[i].F_left_level[k]+1;
                    printf("got here\n");
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
 }



