#include <stdio.h>
#include <string.h>
#include "util.h"
#include "main.h"
#include "globals.h"
#include "read_blif.h"
#include "ff_pack.h"
#include "tree.h"
#include "check_precedence.h"
#include "asap.h"
#include "dummy_tree_place.h"
#include "global_route.h"
#include "check_route.h"
#include "part_place.h"
#include "model.h"
#include "energy.h"
#include "grow.h"
#include "detail_route.h"
#include "tree_energy.h"



// This is used to allocate domains for your routing (scheduling) for assignment 7, 8.
//  Increase if you need it to be larger.
#define SCHEDULE_FACTOR 50

#define BIG_INT 32000

int num_nets, num_blocks;
int alap_time_max;
int *net_timestep;
int num_p_inputs, num_p_outputs, num_luts, num_latches;
struct s_net *net;
struct s_block *block;

static void parse_command (int argc, char *argv[],
			   /*  files */
			   char *blif_file, 
			   char *cluster_file, 
			   char *place_file,
			   char *arch_file,
			   char *global_schedule_file,
			   char *cost_file,
			   int *size,
			   int *pside,
			   /* architecture options */
			   boolean *global_clocks,
			   int *physical_c,
			   int *cluster_size,
			   int *input_cluster_size,
			   int *inputs_per_cluster, 
			   int *clocks_per_cluster, 
			   int *lut_size,
			   /* timing goal */
			   int *makespan, 
			   /* select among multiple approaches */
			   int *approach, 
			   /* samples */
			   boolean *sample_boolean_flag,
			   float *sample_float_flag,
			   /* timing model */
			   float *block_delay,
			   float *intra_cluster_net_delay, 
			   float *inter_cluster_initial_net_delay,
			   float *inter_cluster_per_manhattan_hop_delay,
			   /* energy model */
			   float *intra_cluster_net_energy, 
			   float *inter_cluster_initial_net_energy,
			   float *inter_cluster_per_manhattan_hop_energy
			   );
static int read_int_option (int argc, char *argv[], int iarg);
static float read_float_option (int argc, char *argv[], int iarg);


/* Large pieces of this ere borrowed from Vpack.                    *
 * Vpack credits:
 * Vpack was written by Vaughn Betz at the University of Toronto.   *
 * It is an extension / replacement for the older blifmap program   *
 * used with earlier versions of VPR.                               *
 * Contact vaughn@eecg.utoronto.ca if you wish to use vpack in your *
 * research or if you have questions about its use.                 *
 * All timing enhancements were written by Alexander (Sandy)        *
 * Marquardt at the University of Toronto. Contact                  *
 * arm@eecg.toronto.edu if you have any questions about these       *
 * enhancements                                                     */

int num_input_pins (int iblk) {

/* Returns the number of used input pins on this block. */

 int conn_inps;

 switch (block[iblk].type) {

 case LUT:
    conn_inps = block[iblk].num_nets - 1;   /* -1 for output. */
    break;

 case LUT_AND_LATCH:
    conn_inps = block[iblk].num_nets - 2;  /* -2 for output + clock */
    break;

 case LATCH:
    conn_inps = block[iblk].num_nets - 2;  /* -2 for output + clock */
    break;

 default:
/* This routine should only be used for logic blocks. */
    printf("Error in num_input_pins:  Unexpected block type %d"
           "\n for block %d.  Aborting.\n", block[iblk].type, iblk);
    exit(1);
    break;
 }
 
 return (conn_inps);
}

static void unclustered_stats (int lut_size) {

/* Dumps out statistics on an unclustered netlist -- i.e. it is just *
 * packed into LUT + FF logic blocks, but not local routing from     *
 * output to input etc. is assumed.                                  */

 int iblk, num_logic_blocks;
 int min_inputs_used, min_clocks_used;
 int max_inputs_used, max_clocks_used;
 int summ_inputs_used, summ_clocks_used;
 int inputs_used, clocks_used;

 printf("\nUnclustered Netlist Statistics:\n\n");
 num_logic_blocks = num_blocks - num_p_inputs - num_p_outputs;
 printf("%d Logic Blocks.\n", num_logic_blocks);

 min_inputs_used = lut_size+1;
 min_clocks_used = 2;
 max_inputs_used = -1;
 max_clocks_used = -1;
 summ_inputs_used = 0;
 summ_clocks_used = 0;

 for (iblk=0;iblk<num_blocks;iblk++) {
    if (block[iblk].type == LUT || block[iblk].type == LATCH || 
           block[iblk].type == LUT_AND_LATCH) {
       inputs_used = num_input_pins(iblk);
       if (block[iblk].nets[lut_size+1] != OPEN)
          clocks_used = 1;
       else 
          clocks_used = 0;

       min_inputs_used = min (min_inputs_used, inputs_used);
       max_inputs_used = max (max_inputs_used, inputs_used);
       summ_inputs_used += inputs_used;

       min_clocks_used = min (min_clocks_used, clocks_used);
       max_clocks_used = max (max_clocks_used, clocks_used);
       summ_clocks_used += clocks_used;
    }
 }

 printf("\n\t\t\tAverage\t\tMin\tMax\n");
 printf("Logic Blocks / Cluster\t%f\t%d\t%d\n", 1., 1, 1);
 printf("Used Inputs / Cluster\t%f\t%d\t%d\n", (float) summ_inputs_used /
        (float) num_logic_blocks, min_inputs_used, max_inputs_used);
 printf("Used Clocks / Cluster\t%f\t%d\t%d\n", (float) summ_clocks_used /
        (float) num_logic_blocks, min_clocks_used, max_clocks_used);
 printf("\n");
}



int main (int argc, char *argv[]) {

 char title[] = "\nmain version 0.1 for ESE535 Spring 2015\n"
   " based on: T-Vpack / Vpack Version 4.30\n"
                "Vaughn Betz and (modified by) Alexander(Sandy) Marquardt\n"
                "compiled " __DATE__ ".\n"
                "This code is licensed for non-commercial use only.\n\n";

 char blif_file[BUFSIZE];
 char cluster_file[BUFSIZE];
 char place_file[BUFSIZE];
 char arch_file[BUFSIZE];
 char global_schedule_file[BUFSIZE];
 char cost_file[BUFSIZE];
 char route_tmp_file[BUFSIZE+20];
 int size;
 int pside;
 boolean global_clocks;
 int physical_c;
 int cluster_size;
 int input_cluster_size;
 int inputs_per_cluster;
 int clocks_per_cluster;
 int lut_size;
 int makespan;
 int approach;
 boolean sample_boolean_flag;
 float sample_float_flag;
 float block_delay;
 float intra_cluster_net_delay; 
 float inter_cluster_initial_net_delay;
 float inter_cluster_per_manhattan_hop_delay;
 float intra_cluster_net_energy;
 float inter_cluster_initial_net_energy;
 float inter_cluster_per_manhattan_hop_energy;

 boolean *is_clock;      /* [0..num_nets-1] TRUE if a clock. */

 int total_ios;
 int total_luts;

 int violations;
 int unplaced;
 int precedence_violations;

 float delay=-1;

 int lb_makespan=-1;
 int makespan_achieved=-1;

 printf("%s",title);

 parse_command (argc, argv, 
		blif_file, 
		cluster_file, 
		place_file,
		arch_file,
		global_schedule_file,
		cost_file,
		&size,
		&pside,
		&global_clocks,
		&physical_c, 
		&cluster_size, 
		&input_cluster_size,
		&inputs_per_cluster, 
		&clocks_per_cluster, 
		&lut_size,
		&makespan,
		&approach,
		&sample_boolean_flag, 
		&sample_float_flag,
		&block_delay, 
		&intra_cluster_net_delay,
		&inter_cluster_initial_net_delay, 
		&inter_cluster_per_manhattan_hop_delay, 
		&intra_cluster_net_energy,
		&inter_cluster_initial_net_energy,
		&inter_cluster_per_manhattan_hop_energy
		);

 
 read_blif (blif_file, lut_size);
 echo_input (blif_file, lut_size, "input.echo");

 pack_luts_and_ffs (lut_size);

 /* verbose */
 printf("Packed LUTs, now compressing netlist\n");

 compress_netlist (lut_size);

 /* verbose */
 printf("Compressed.  Now allocating clock.\n");

/* NB:  It's important to mark clocks and such *after* compressing the   *
 * netlist because the net numbers, etc. may be changed by removing      *
 * unused inputs and latch folding.                                      */

 is_clock = alloc_and_load_is_clock (global_clocks, lut_size);

 printf("\nAfter packing to LUT+FF Logic Blocks:\n");
 printf("LUT+FF Logic Blocks: %d.  Total Nets: %d.\n", num_blocks - 
       num_p_outputs - num_p_inputs, num_nets);


/* Uncomment line below if you want a dump of compressed netlist. */
/* echo_input (blif_file, lut_size, "packed.echo"); */


 if (size==0)
   {
     printf("size not given, computing.\n");
      
     size=(num_blocks+cluster_size-1)/cluster_size;

     printf("\tsize=%d\n",size);
   }


 printf("Fill Ratio: %f\n",((double)num_blocks/(cluster_size*size)));

 if (num_blocks>(cluster_size*size))
   {
     fprintf(stderr,"Tree size and clustering options too small.\n");
     fprintf(stderr,"\tCannot continue.  Exiting.");
     exit(2);

   }

 /* sanity check makespan */
 
 lb_makespan=asap_delay();
 alap_time_max=lb_makespan;
 // print_level_assignment(); // for debugging
 if (makespan<=0)
   makespan=lb_makespan; // CONSIDER: like pside, something to add?

 if (lb_makespan<makespan)
   {
     fprintf(stderr,"Makespan target %d is not feasible.  Lower bound is %d.\n",makespan,lb_makespan);
     fprintf(stderr,"\tCannot continue.  Exiting.");
     exit(3);
   }
 else
   {
     fprintf(stdout,"Makespan lower bound is %d.\n",lb_makespan);
   }

 /* allocate placement structure */

 
/* case statement based on approach here */

  int *growth_schedule;
  int height=ilog2(size);
  int growth_schedule_length=height+1;
  int i;

 switch (approach)
   {
   case 0:
     /* Dummy case for starters */
     printf("Case 0: Doing nothing.  Should complain things are unplaced\n");
     break;
   case 1:
     printf("Case 1: Dummy tree placer.\n");
     printf("\tShould work, but use excessive number of PEs.\n");
     dummy_tree_place(size,cluster_size,is_clock,global_clocks);
     break;
   case 2:
     printf("Case 2: Dummy tree placer (C=5, P=0).\n");
     growth_schedule=(int *)malloc(growth_schedule_length*sizeof(int));
     growth_schedule[0]=5;
     for (i=1;i<growth_schedule_length;i++)
       growth_schedule[i]=1;
     dummy_tree_place_schedule(growth_schedule,height,cluster_size,is_clock,global_clocks);
     break;
   case 3:
     printf("Case 3: Single Partition only, total cut.\n");
     part_place(size,cluster_size,FALSE,FALSE,FALSE,is_clock,global_clocks);
     calculate_and_write_top_cut(size,cost_file,FALSE);
     /* ...and that's all we want to do for this case */
     /* cleanup */
     free(is_clock);
     free_tree();
     exit(0);
     break;
   case 4:
     printf("Case 4: Single Partition only, level cut.\n");
     part_place(size,cluster_size,FALSE,TRUE,FALSE,is_clock,global_clocks);
     break;
   case 5:
     printf("Case 5: Recursive Partition only, total cut.\n");
     part_place(size,cluster_size,TRUE,FALSE,FALSE,is_clock,global_clocks);
     break;
   case 6:
     printf("Case 6: Recursive Partition only, level cut.\n");
     part_place(size,cluster_size,TRUE,TRUE,FALSE,is_clock,global_clocks);
     break;
     /* ADD OTHER CASES AS NECESSARY */
   default:
     printf("Unknown case");
   }

 /* print_tree(); */  // see if it built a reasonable tree....



 /* check everything placed */
 unplaced=count_unplaced(REPORT_UNPLACED_NAME_TYPE_LOC); 
 /* less verbose version */
 /* unplaced=count_unplaced(REPORT_UNPLACED_NONE); */


 /* check obey precedence constraints */
 precedence_violations=check_precedence(TRUE);
 /* TRUE here tells it to print out precedence violations;
    make it FALSE if you don't want to see those printed out. */


 /* check makespan */
 makespan_achieved=find_max_timestep();


 /* check clusters not overused */
 violations=count_placement_violations(cluster_size,input_cluster_size,
				       lut_size, inputs_per_cluster,
				       REPORT_VIOLATIONS_LOCATION_COUNT);


 if ((unplaced>0) || (precedence_violations>0) || (violations>0) || (makespan_achieved>makespan))
   {
     fprintf(stderr,"Final placement/clustering is illegal\n");
     fprintf(stderr,"\t(%d items unplaced)\n",unplaced);
     fprintf(stderr,"\t(%d precedence violations)\n",precedence_violations);
     fprintf(stderr,"\t(%d capacity violations)\n",violations);
     fprintf(stderr,"\t(makespan achieved %d)\n",makespan_achieved);
     fprintf(stderr,"\tNot writing cluster; placement file may be bogus\n");
     /* but you may want to have it dump details explaining the problem... */
   }
 else
   {

     /* write clusters */
     print_clusters(cluster_file,cluster_size,inputs_per_cluster,
		    clocks_per_cluster,lut_size,global_clocks,
		    FALSE, is_clock);

     /* write placement */
     print_place(place_file,cluster_file,arch_file);
     print_by_timestep(global_schedule_file);

     /* route */
     global_route(is_clock,global_clocks,FALSE); // change to TRUE to make verbose

     // DEBUG
     //sprintf(route_tmp_file,"%s.rinfo",blif_file);
     //dump_parent_domains(route_tmp_file);

     /* check that global route worked */
     int route_fails=valid_routes(is_clock,global_clocks,FALSE); // change to TRUE to make verbose
     if (route_fails>0)
       {
	 fprintf(stderr,"Unrouted Nets: %d\n",route_fails);
       }
     else
       {
	 fprintf(stdout,"global route succeeded.\n");

	 // replacing this with energy cost below
	 // calls to assign2 cost calculations
	 // calculate_and_print_costs(blif_file,cost_file,(makespan+1),growth_schedule_length);

	 // (1) get growth schedule from placement and global route
	 int *glevel=grow_level(growth_schedule_length,(makespan+1));
	 int expand=(glevel[0]+physical_c-1)/physical_c;
	 glevel[0]=physical_c; // use this instead of what growth level schedule calculates

	 // (2) free old tree
	 free_tree_no_block_cleanup(); // keep the placements so can replace in new tree


	 // (3) rebuild tree to growth schedule
	 int maxwaves=expand*(makespan+1)*SCHEDULE_FACTOR; // expect you can schedule in 2, making this 3 initially, increase as needed
	 allocate_tree_domains(glevel,
			       height,
			       maxwaves,
			       cluster_size,// blocks per PE
			       cluster_size// io limits, not currently used
			       );

	 // (4) replace into newly created tree
	 replace();

	 // (4) call detail route
     printf("Calling detail route\n");
	 detail_route(is_clock,global_clocks,maxwaves,FALSE); // change to TRUE to make verbose

	 // DEBUG
	 int used_waves=get_max_domain_used(height,maxwaves);
	 dump_detail_route_domains("detail_route.rinfo",(used_waves+1));

	 /* check that global route worked */
	 int route_fails=valid_detail_routes(is_clock,global_clocks,maxwaves,FALSE); // change to TRUE to make verbose
	 if (route_fails>0)
	   {
	     fprintf(stderr,"Unrouted Nets: %d\n",route_fails);
	   }
	 else
	   {
	     fprintf(stdout,"detail route succeeded.\n");
	     
	     // (5) gather up stats and call energy function and record results
	     int use_growth_length=growth_schedule_length;
	     int *plevel=(int *)malloc(sizeof(int)*use_growth_length);
	     plevel[0]=physical_c;
	     // physical growth of p=0.5
	     if (use_growth_length>1)
	       plevel[1]=2;
	     for (i=2;i<use_growth_length;i++)
	       if (plevel[i-1]==1)
		 plevel[i]=2;
	       else
		 plevel[i]=1;
	     calculate_and_print_energy(blif_file,cost_file,(makespan+1),maxwaves,use_growth_length,cluster_size,lut_size,glevel,plevel);
	 
	   }
       }
     
   }


     free (is_clock);

 /* cleanup */
 free_tree();

 printf("\nmain complete.\n\n");
 return (0);
}




static void parse_command (int argc, char *argv[],
			   /*  files */
			   char *blif_file, 
			   char *cluster_file, 
			   char *place_file,
			   char *arch_file,
			   char *global_schedule_file,
			   char *cost_file,
			   int *size,
			   int *pside,
			   /* architecture options */
			   boolean *global_clocks, 
			   int *physical_c,
			   int *cluster_size,
			   int *input_cluster_size,
			   int *inputs_per_cluster, 
			   int *clocks_per_cluster, 
			   int *lut_size,
			   /* target makespan */
			   int *makespan,
			   /* select among multiple approaches */
			   int *approach, 
			   /* samples */
			   boolean *sample_boolean_flag,
			   float *sample_float_flag,
			   /* timing model */
			   float *block_delay,
			   float *intra_cluster_net_delay, 
			   float *inter_cluster_initial_net_delay,
			   float *inter_cluster_per_manhattan_hop_delay,
			   /* variance for statistical timing model */
			   float *intra_cluster_net_energy,
			   float *inter_cluster_initial_net_energy,
			   float *inter_cluster_per_manhattan_hop_energy
			   )

{


/* Parse the command line to determine user options. */

 int i;
 boolean inputs_per_cluster_set;

 if (argc < 7) {
    printf("Usage:  main input.blif output.net output.place architecture.arch output.sched output.cost\n");
    printf("\t[-s <int>] [-pside <int>]\n");
    printf("\t[-lut_size <int>] [-cluster_size <int>] \n");
    printf("\t[-physical_c <int>]\n");
    printf("\t[-input_cluster_size <int>] [-inputs_per_cluster <int>]\n");
    printf("\t[-clocks_per_cluster <int>] [-global_clocks on|off]\n");
    printf("\t[-makespan <int>] \n");
    printf("\t[-approach <int>] \n");
    printf("\t[-sample_boolean_flag on|off]  [-sample_float_flag <float>]\n");
    printf("\t[-block_delay <float>]\n");
    printf("\t[-intra_cluster_net_delay <float>] \n");
    printf("\t[-inter_cluster_initial_net_delay <float>] \n");
    printf("\t[-inter_cluster_per_manhattan_hop_delay <float>] \n");
    printf("\t[-intra_cluster_net_energy <float>] \n");
    printf("\t[-inter_cluster_initial_net_energy <float>] \n");
    printf("\t[-inter_cluster_per_manhattan_hop_energy <float>] \n");
    exit(1);
 }

/* Set defaults. */

 *lut_size = 4;
 *global_clocks = TRUE;
 *physical_c = 1;
 *cluster_size = 1;
 *input_cluster_size=1;
 *inputs_per_cluster = -1;  /* Dummy.  Need other params to set default. */
 inputs_per_cluster_set = FALSE;
 *clocks_per_cluster = 1;

 *size=0;
 *pside=0;

 *makespan=0;

 *sample_boolean_flag = TRUE;
 *sample_float_flag=1.0;


 *block_delay=0.1;
 *intra_cluster_net_delay=0.1;
 *inter_cluster_initial_net_delay=0.1;
 *inter_cluster_per_manhattan_hop_delay=1.0;

 /* initializing to determininstic = no variance */
 *intra_cluster_net_energy=0.0;
 *inter_cluster_initial_net_energy=0.0;
 *inter_cluster_per_manhattan_hop_energy=0.0;


/* Start parsing the command line.  First three arguments are mandatory. */

 strncpy (blif_file, argv[1], BUFSIZE);
 strncpy (cluster_file, argv[2], BUFSIZE);
 strncpy (place_file, argv[3], BUFSIZE);
 strncpy (arch_file, argv[4], BUFSIZE);
 strncpy (global_schedule_file, argv[5], BUFSIZE);
 strncpy (cost_file, argv[6], BUFSIZE);
 i = 7;


/* Now get any optional arguments. */

 while (i < argc) {

   /* blah, shouldn't command-line parser deal with these ? */
   if (strcmp(argv[i], "\t")==0)
     {
       i++;
       continue;
     }
   if (strcmp(argv[i], "\r")==0)
     {
       i++;
       continue;
     }
   if (strcmp(argv[i], "\n")==0)
     {
       i++;
       continue;
     }

   
    if (strcmp (argv[i], "-lut_size") == 0) {
    
       *lut_size = read_int_option (argc, argv, i);

       if (*lut_size < 2 || *lut_size > MAXLUT) {
          printf("Error:  lut_size must be between 2 and MAXLUT (%d).\n",
                   MAXLUT);
          exit (1);
       }
       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-global_clocks") == 0) {
       if (argc <= i+1) {
          printf ("Error:  -global_clocks option requires a string parameter."
                  "\n"); 
          exit (1);    
       } 
       if (strcmp(argv[i+1], "on") == 0) {
          *global_clocks = TRUE;
       } 
       else if (strcmp(argv[i+1], "off") == 0) {
          *global_clocks = FALSE;
       } 
       else {
          printf("Error:  -global_clocks must be on or off.\n");
          exit (1);
       } 
 
       i += 2;
       continue;
    }



    if (strcmp (argv[i],"-sample_boolean_flag") == 0) {
       if (argc <= i+1) {
          printf ("Error:  -sample_boolean_flag expects a string argument."
                  "\n");
          exit (1);
       }
       if (strcmp(argv[i+1], "on") == 0) {
          *sample_boolean_flag = TRUE;
       }
       else if (strcmp(argv[i+1], "off") == 0) {
          *sample_boolean_flag = FALSE;
       }
       else {
          printf("Error:  -hill_climbing must be on or off.\n");
          exit (1);
       }
 
       i += 2;
       continue;
    }


    if (strcmp (argv[i],"-sample_float_flag") == 0) {
       *sample_float_flag = read_float_option (argc, argv, i);

       if (*sample_float_flag > 1.0 || *sample_float_flag < 0.0) {
          printf("Error:  sample_float_flag must be between 0 and 1.\n");
          exit (1);
       }

       i += 2;
       continue;
    }



    if (strcmp (argv[i],"-block_delay") == 0) {
       *block_delay = read_float_option (argc, argv, i);
       i += 2;
       continue;
    }


    if (strcmp (argv[i],"-intra_cluster_net_delay") == 0) {
       *intra_cluster_net_delay = read_float_option (argc, argv, i);
       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-inter_cluster_initial_net_delay") == 0) {
       *inter_cluster_initial_net_delay = read_float_option (argc, argv, i);
       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-inter_cluster_per_manhattan_hop_delay") == 0) {
       *inter_cluster_per_manhattan_hop_delay = read_float_option (argc, argv, i);
       i += 2;
       continue;
    }



    if (strcmp (argv[i],"-intra_cluster_net_energy") == 0) {
       *intra_cluster_net_energy = read_float_option (argc, argv, i);
       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-inter_cluster_initial_net_energy") == 0) {
       *inter_cluster_initial_net_energy = read_float_option (argc, argv, i);
       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-inter_cluster_per_manhattan_hop_energy") == 0) {
       *inter_cluster_per_manhattan_hop_energy = read_float_option (argc, argv, i);
       i += 2;
       continue;
    }



    if (strcmp (argv[i],"-cluster_size") == 0) {
       *cluster_size = read_int_option (argc, argv, i);

       if (*cluster_size < 1) {
          printf("Error:  cluster_size must be greater than 0.\n");
          exit (1);
       }

       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-physical_c") == 0) {
       *physical_c = read_int_option (argc, argv, i);

       if (*physical_c < 1) {
          printf("Error:  physical_c must be greater than 0.\n");
          exit (1);
       }

       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-input_cluster_size") == 0) {
       *input_cluster_size = read_int_option (argc, argv, i);

       if (*input_cluster_size < 1) {
          printf("Error:  input_cluster_size must be greater than 0.\n");
          exit (1);
       }

       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-inputs_per_cluster") == 0) {
       *inputs_per_cluster = read_int_option (argc, argv, i);
       inputs_per_cluster_set = TRUE;

       /* Do sanity check after cluster_size and lut_size known. */

       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-clocks_per_cluster") == 0) {
       *clocks_per_cluster = read_int_option (argc, argv, i);

       /* Do sanity check after cluster_size known. */

       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-makespan") == 0) {
       *makespan = read_int_option (argc, argv, i);

       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-approach") == 0) {
       *approach = read_int_option (argc, argv, i);

       /* default in case statement is catchall for out of range */

       i += 2;
       continue;
    }

    if (strcmp (argv[i],"-s") == 0) {
       *size = read_int_option (argc, argv, i);

       i += 2;
       continue;
    }


    if (strcmp (argv[i],"-pside") == 0) {
       *pside = read_int_option (argc, argv, i);
       i += 2;
       continue;
    }
    

    printf("Unrecognized option: %s.  Aborting.\n",argv[i]);
    exit(1);

 }   /* End of options while loop. */
 
/* Assign new default values if required. */

 if (!inputs_per_cluster_set) 
    *inputs_per_cluster = *lut_size * *cluster_size;


 /*additional santiy checks*/

 if (*inputs_per_cluster < *lut_size || *inputs_per_cluster > *cluster_size
            * *lut_size) {
    printf("Error:  Number of input pins must be between lut_size and "
       "cluster_size * lut_size.\n");
    exit (1);
 }

 if (*clocks_per_cluster < 1 || *clocks_per_cluster > *cluster_size) {
    printf("Error:  Number of clock pins must be between 1 and "
         "cluster_size.\n");
    exit(1);
 }




/* Echo back options. */

 printf("Selected options:\n\n");
 printf("\tLut Size: %d inputs\n", *lut_size);
 printf("\tCluster Size: %d (LUT+FF) logic block(s) / cluster\n", 
      *cluster_size);
 printf("\tPhysical C: %d\n", *physical_c);
 printf("\tInput Cluster Size: %d\n", 
      *input_cluster_size);
 printf("\tDistinct Input Pins Per Cluster: %d\n", *inputs_per_cluster);
 printf("\tClock Pins Per Cluster: %d\n", *clocks_per_cluster);
 printf("\tClocks Routed Via Dedicated Resource: %s\n",
      (*global_clocks) ? "Yes" : "No");
 printf("\tSample Boolean Flag: %s\n",
      (*sample_boolean_flag) ? "Yes" : "No");
 printf("\tSample Float Flag: %f\n",
	(*sample_float_flag));
printf("\ttree size %d\n",*size);
printf("\tMakespan: %d\n",*makespan);
printf("\tApproach: %d\n",*approach);
printf("\tBlock Delay %f\n",
       *block_delay);
printf("\tIntra Cluster Net Delay %f (energy %f)\n",
       *intra_cluster_net_delay,*intra_cluster_net_energy);
printf("\tInter Cluster Initial Net Delay %f (energy %f)\n",
       *inter_cluster_initial_net_delay,
       *inter_cluster_initial_net_energy);
printf("\tInter Cluster Per Manhattan Hop Delay %f (energy %f)\n",
       *inter_cluster_per_manhattan_hop_delay,
       *inter_cluster_per_manhattan_hop_energy);


 printf("\n\n");
}


static int read_int_option (int argc, char *argv[], int iarg) {

/* This routine returns the value in argv[iarg+1].  This value must exist *
 * and be an integer, or an error message is printed and the program      *
 * exits.                                                                 */
 
 int value, num_read;
 
 num_read = 0;
 
/* Does value exist for this option? */
 
 if (argc > iarg+1)
    num_read = sscanf(argv[iarg+1],"%d",&value);
 
/* Value exists and was a proper int? */
 
 if (num_read != 1) {
    printf("Error:  %s option requires an integer parameter.\n\n", argv[iarg]);
    exit(1);
 }
 
 return (value);
}


static float read_float_option (int argc, char *argv[], int iarg) {

/* This routine returns the value in argv[iarg+1].  This value must exist *
 * and be an integer, or an error message is printed and the program      *
 * exits.                                                                 */
 
 float value;
 int num_read;
 
 num_read = 0;
 
/* Does value exist for this option? */
 
 if (argc > iarg+1)
    num_read = sscanf(argv[iarg+1],"%f",&value);
 
/* Value exists and was a proper int? */
 
 if (num_read != 1) {
    printf("Error:  %s option requires an integer parameter.\n\n", argv[iarg]);
    exit(1);
 }
 
 return (value);
}
