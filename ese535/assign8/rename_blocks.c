#include "globals.h"
#include "rename_blocks.h"
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

FILE *vars;
FILE *deref;
FILE *optim;

void process_block_list(void)
{
    int i;
    int j;
    int index=0;
    vars=fopen("renamed_list.txt", "wb");
    deref=fopen("deref_list.txt", "wb");
    optim=fopen("output.pb", "wb");

    fprintf(optim, "min:");
    fprintf(optim, " ;\n");
    int blocks_not_clocks=0;
    int no_clocks[num_blocks];
    int whichNet;

    // Remove clocks, we won't be placing them
    for(i=0; i<num_blocks; i++)
    {
        if(block[i].loc!=GLOBAL_CLOCK_LOC)
        {
            blocks_not_clocks++;
            no_clocks[blocks_not_clocks]=i;
        }
    }

    // Find the sources and associate nets so we can locate the sinks
    // We will use this for creating the objective function.
    for(i=0; i<blocks_not_clocks+1; i++)
    {
        if(block[no_clocks[i]].type!=OUTPAD)
        {
            whichNet=block[no_clocks[i]].nets[0];
            src=no_clocks[i];
        }
    }

    // Create the all nodes represented constraint (no replicants)
    for(i=0; i<blocks_not_clocks+1; i++)
    {
        index=i*j;
        for(j=0; j<8; j++)
        { 
            fprintf(vars,"x%d\n", (index+j));
            fprintf(deref,"%s   x%d   leaf: %d\n", block[no_clocks[i]].name, index+j, j);
            fprintf(optim,"+1 x%d ", (index+j));
            
        }
        
        fprintf(optim," = 1 ;\n");
        //fprintf(deref,"\n");
    }

    index=0;

    // Create the balanced partitions constraint
    for(j=0; j<8; j++)
    {
        //index=i*j;
        for(i=0; i<blocks_not_clocks+1; i++)
        {
            fprintf(vars,"x%d\n", (j+i*8));
            fprintf(deref,"%s   x%d   leaf: %d\n", block[no_clocks[i]].name, index+j, j);
            fprintf(optim,"+1 x%d ", (j+i*8));
        }
        fprintf(optim," <= 4 ;\n");
    }
}