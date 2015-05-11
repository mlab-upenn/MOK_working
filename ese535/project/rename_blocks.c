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
FILE *header;
void process_block_list(void)
{
    int i;
    int j;
    int index=0;
    vars=fopen("renamed_list.txt", "wb");
    deref=fopen("deref_list.txt", "wb");
    optim=fopen("output.pb", "wb");
    header=fopen("header.pb", "wb");


    
    int blocks_not_clocks=0;
    int no_clocks[num_blocks];
    int whichNet;
    int size=(num_blocks+4-1)/4;
    int num_leaves=pow(2,ilog2(size));
    int variable=0;
    int constraint=0;
    int product=0;
    int sizeproduct=5;

    // Remove clocks, we won't be placing them
    for(i=0; i<num_blocks; i++)
    {
        if(block[i].loc!=GLOBAL_CLOCK_LOC)
        {
            blocks_not_clocks++;
            no_clocks[blocks_not_clocks]=i;
        }
    }

    ListPtr *x_vals=malloc(sizeof(ListPtr)*blocks_not_clocks);

    for(i=0; i<num_blocks; i++)
    {
        index=i*num_leaves;
        x_vals[i]=NULL;
        x_vals[i]=malloc(sizeof(ListPtr)*num_blocks);
        ListPtr temp=x_vals[i];
        temp->nextPtr=NULL;
        temp->cellNumber=-1;
        for(j=0; j<num_leaves; j++)
        {
            insert(&x_vals[i], index+j+1);
            variable++;
        }
    }

    
    fprintf(optim, "min:");

    // Find the sources and associate nets so we can locate the sinks
    // We will use this for creating the objective function.
    /*for(i=0; i<num_blocks; i++)
    {
        if(block[i].type!=OUTPAD && block[i].loc!=GLOBAL_CLOCK_LOC)
        {
            int whichNet=block[i].nets[0];
            int src=i;

            int start=1;
            if (block[src].type==OUTPAD)
                start=0;

            ListPtr nextSink=NULL;
            ListPtr currentSink=x_vals[net[whichNet].pins[j]];
            ListPtr nextSrc=NULL;
            ListPtr currentSrc=x_vals[src];

            while(currentSink!=NULL)
            {
                    if(current->cellNumber>0)
                    {
                        product++;
                        fprintf(optim, "+1 ~x%d", x_vals[src]->cellNumber);
                        fprintf(optim, " x%d ", current->cellNumber);
                    }
                    
                    current=current->nextPtr;

            }
            
        }
    }*/
  
    fprintf(optim, " ;\n");

    // Create the all nodes represented constraint (no replicants)
    for(i=0; i<blocks_not_clocks+1; i++)
    {
        index=i*j;
        for(j=0; j<num_leaves; j++)
        { 
            
            fprintf(vars,"x%d\n", (index+j));
            fprintf(deref,"%s   x%d   leaf: %d\n", block[no_clocks[i]].name, index+j, j);
            fprintf(optim,"+1 x%d ", (index+j+1));
            
        }
        
        fprintf(optim," = 1 ;\n");
        constraint++;
        //fprintf(deref,"\n");
    }

    index=0;

    // Create the balanced partitions constraint
    for(j=0; j<num_leaves; j++)
    {
        //index=i*j;
        for(i=0; i<blocks_not_clocks+1; i++)
        { 
            
            fprintf(vars,"x%d\n", (j+i*8));
            fprintf(deref,"%s   x%d   leaf: %d\n", block[no_clocks[i]].name, index+j, j);
            fprintf(optim,"-1 x%d ", (j+i*8));
        }
        fprintf(optim," >= -4 ;\n");
        constraint++;
    }

    fprintf(optim,"x1 = 1 ;\n");
    constraint++;

sizeproduct=2*product;
fprintf(header,"* #variable= %d #constraint= %d #product= %d sizeproduct= %d\n", variable, constraint, product, sizeproduct);
fprintf(header, "*\n* comments\n*\n*\n");
fclose(header);
fclose(optim);

FILE *mytest=fopen("header.pb", "a");
FILE *test=fopen("output.pb", "r");

if (mytest!=NULL)
{
    printf("opened header\n");
}

if (test!=NULL)
{
    printf("opened optim\n");
}


char buffer[100];
printf("about to append\n");
rewind(optim);
printf("%d\n", feof(optim));

while(feof(test)==0)
{
    fread(buffer, sizeof(char), 100,  test);
    printf("read in a block\n");
    fwrite(buffer, sizeof(char), 100 ,mytest);

}
//fclose(header);
//fclose(optim);
}
