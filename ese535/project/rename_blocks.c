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

    int no_clocks[num_blocks];
    int whichNet;
    int size=(num_blocks+4-1)/4;
    int num_leaves=pow(2,ilog2(size));
    int variable=0;
    int constraint=0;
    int product=0;
    int sizeproduct=0;

    ListPtr *x_vals=malloc(sizeof(ListPtr)*num_blocks);

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

    
    //fprintf(optim, "min:");

    // Find the sources and associate nets so we can locate the sinks
    // We will use this for creating the objective function.
    for(i=0; i<num_blocks; i++)
    {
        if(block[i].type!=OUTPAD && block[i].loc!=GLOBAL_CLOCK_LOC)
        {
            int whichNet=block[i].nets[0];
            int src=i;

            int start=1;
            if (block[src].type==OUTPAD)
                start=0;

            for(j=start; j<net[whichNet].num_pins; j++)
            {
                ListPtr nextSrc=NULL;
                ListPtr currentSrc=x_vals[src];
                ListPtr nextSink=NULL;
                ListPtr currentSink=x_vals[net[whichNet].pins[j]];
                printf("got here\n");

                while(currentSrc!=NULL && currentSink!=NULL)
                {

                        if(currentSrc->cellNumber>0 && currentSink->cellNumber>0)
                        {
                            printf("Current source= x%d", currentSrc->cellNumber);
                            product++;
                            fprintf(optim, "-1 ~x%d", currentSrc->cellNumber);
                            fprintf(optim, " x%d ", currentSink->cellNumber);
                            
                        }

                    currentSink=currentSink->nextPtr;
                    currentSrc=currentSrc->nextPtr;
                }

            }
            
        }
    }

  
    fprintf(optim, " >= -100000;\n");
    constraint++;

    // Create the all nodes represented constraint (no replicants)
    for(i=0; i<num_blocks; i++)
    {
        index=i*j;
        if(block[i].type!=OUTPAD && block[i].loc!=GLOBAL_CLOCK_LOC)
        {
            for(j=0; j<num_leaves; j++)
            { 

                fprintf(vars,"x%d\n", (index+j));
                fprintf(deref,"%s   x%d   leaf: %d\n", block[i].name, index+j, j);
                fprintf(optim,"+1 x%d ", (index+j+1));

            }

            fprintf(optim," = 1 ;\n");
            constraint++;
        }
    }

    index=0;

    // Create the balanced partitions constraint
    for(j=0; j<num_leaves; j++)
    {
        for(i=0; i<num_blocks; i++)
        { 
            if(block[i].type!=OUTPAD && block[i].loc!=GLOBAL_CLOCK_LOC)
            {
                fprintf(vars,"x%d\n", (j+i*8));
                fprintf(deref,"%s   x%d   leaf: %d\n", block[i].name, index+j, j);
                fprintf(optim,"-1 x%d ", (j+i*num_leaves)+1);
            }
        }
        fprintf(optim," >= -4 ;\n");
        constraint++;
    }

    fprintf(optim, "+1 x1 = 0 ;\n");
    constraint++;


    sizeproduct=2*product;
    fprintf(header,"* #variable= %d #constraint= %d #product= %d sizeproduct= %d\n", variable, constraint, product, sizeproduct);
    fprintf(header, "*\n* comments\n*\n*\n");
    fclose(header);
    fclose(optim);

    FILE *mytest=fopen("header.pb", "a");
    FILE *test=fopen("output.pb", "r");

    char buffer[100];
    printf("about to append\n");
    rewind(optim);
    printf("%d\n", feof(optim));

    while(feof(test)==0)
    {
        int num;
        num=fread(buffer, sizeof(char), 100,  test);
        printf("read in a block\n");
        fwrite(buffer, sizeof(char), num,mytest);
    }

}
