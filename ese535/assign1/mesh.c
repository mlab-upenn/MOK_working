/* written by andre 2/08 ... updated 1/11 */

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "mesh.h"
#include "sched_main.h"
#include "globals.h"
#include "heapsort.h"
#include "output_clustering.h"


#define position(x,y) (mesh_width*(y)+x)

struct mesh_position *mesh;
int mesh_width;  /* width of mesh including io*/
int mesh_height; /* height of mesh including io*/



void allocate_mesh(int x, 
		   int y, 
		   int luts_per_cluster, 
		   int ios_per_iocluster)
{

  int i,j,k;

  mesh_width=(x+2);
  mesh_height=(y+2); 
  mesh=(struct mesh_position *)malloc(mesh_width*mesh_height*sizeof(struct mesh_position)); 

  /* invalid slots -- nothing goes in the 4 corners */

  mesh[position(0,0)].type=INVALIDSLOT;
  mesh[position(0,0)].used_slots=0;
  mesh[position(0,0)].allocated_slots=0;
  mesh[position(0,0)].slots=(int *)NULL;
  mesh[position(0,0)].timesteps=(int *)NULL;


  mesh[position(0,y+1)].type=INVALIDSLOT;
  mesh[position(0,y+1)].used_slots=0;
  mesh[position(0,y+1)].allocated_slots=0;
  mesh[position(0,y+1)].slots=(int *)NULL;
  mesh[position(0,y+1)].timesteps=(int *)NULL;


  mesh[position(x+1,0)].type=INVALIDSLOT;
  mesh[position(x+1,0)].used_slots=0;
  mesh[position(x+1,0)].allocated_slots=0;
  mesh[position(x+1,0)].slots=(int *)NULL;
  mesh[position(x+1,0)].timesteps=(int *)NULL;


  mesh[position(x+1,y+1)].type=INVALIDSLOT;
  mesh[position(x+1,y+1)].used_slots=0;
  mesh[position(x+1,y+1)].allocated_slots=0;
  mesh[position(x+1,y+1)].slots=(int *)NULL;
  mesh[position(x+1,y+1)].timesteps=(int *)NULL;

  
  /* these are the LUT slots */
  for (i=1;i<=x;i++)
    for (j=1;j<=y;j++)
      {

	mesh[position(i,j)].type=LUTFF_SLOT;
	mesh[position(i,j)].used_slots=0;
	mesh[position(i,j)].slots=(int *)malloc(luts_per_cluster*sizeof(int));
	mesh[position(i,j)].timesteps=(int *)malloc(luts_per_cluster*sizeof(int));
	for (k=0;k<luts_per_cluster;k++)
	  {
	    mesh[position(i,j)].slots[k]=EMPTY_SLOT;
	    mesh[position(i,j)].timesteps[k]=UNASSIGNED_TIMESTEP;
	  }
	mesh[position(i,j)].allocated_slots=luts_per_cluster;
	
      }

  /* IO slots */

  /* top and bottom */
  for (i=1;i<=x;i++)
    {

	mesh[position(i,0)].type=IOSLOT;
	mesh[position(i,0)].used_slots=0;
	mesh[position(i,0)].slots=(int *)malloc(ios_per_iocluster*sizeof(int));
	mesh[position(i,0)].timesteps=(int *)malloc(ios_per_iocluster*sizeof(int));
	for (k=0;k<ios_per_iocluster;k++)
	  {
	    mesh[position(i,0)].slots[k]=EMPTY_SLOT;
	    mesh[position(i,0)].timesteps[k]=UNASSIGNED_TIMESTEP;
	  }
	mesh[position(i,0)].allocated_slots=ios_per_iocluster;



	mesh[position(i,y+1)].type=IOSLOT;
	mesh[position(i,y+1)].used_slots=0;
	mesh[position(i,y+1)].slots=(int *)malloc(ios_per_iocluster*sizeof(int));
	mesh[position(i,y+1)].timesteps=(int *)malloc(ios_per_iocluster*sizeof(int));
	for (k=0;k<ios_per_iocluster;k++)
	  {
	    mesh[position(i,y+1)].slots[k]=EMPTY_SLOT;
	    mesh[position(i,y+1)].slots[k]=UNASSIGNED_TIMESTEP;
	  }
	mesh[position(i,y+1)].allocated_slots=ios_per_iocluster;
    }


  /* left and right */
  for (j=1;j<=y;j++)
    {

	mesh[position(0,j)].type=IOSLOT;
	mesh[position(0,j)].used_slots=0;
	mesh[position(0,j)].slots=(int *)malloc(ios_per_iocluster*sizeof(int));
	mesh[position(0,j)].timesteps=(int *)malloc(ios_per_iocluster*sizeof(int));
	for (k=0;k<ios_per_iocluster;k++)
	  {
	    mesh[position(0,j)].slots[k]=EMPTY_SLOT;
	    mesh[position(0,j)].slots[k]=UNASSIGNED_TIMESTEP;
	  }
	mesh[position(0,j)].allocated_slots=ios_per_iocluster;


	mesh[position(x+1,j)].type=IOSLOT;
	mesh[position(x+1,j)].used_slots=0;
	mesh[position(x+1,j)].slots=(int *)malloc(ios_per_iocluster*sizeof(int));
	mesh[position(x+1,j)].timesteps=(int *)malloc(ios_per_iocluster*sizeof(int));
	for (k=0;k<ios_per_iocluster;k++)
	  {
	    mesh[position(x+1,j)].slots[k]=EMPTY_SLOT;
	    mesh[position(x+1,j)].slots[k]=UNASSIGNED_TIMESTEP;
	  }
	mesh[position(x+1,j)].allocated_slots=ios_per_iocluster;
    }

}

int get_mesh_width()
{
  return(mesh_width);
}

int get_mesh_height()
{
  return(mesh_height);
}


void free_mesh()
{
  int i,j;
  for (i=0;i<mesh_width;i++)
    for (j=0;j<mesh_height;j++)
      {
	if (mesh[position(i,j)].slots!=(int *)NULL)
	  free(mesh[position(i,j)].slots);
	mesh[position(i,j)].slots=(int *)NULL;
	if (mesh[position(i,j)].timesteps!=(int *)NULL)
	  free(mesh[position(i,j)].timesteps);
	mesh[position(i,j)].timesteps=(int *)NULL;
      }
  free(mesh);
  mesh=(struct mesh_position *)NULL;

  // remove placement information in block since mesh has gone away
  for (i=0;i<num_blocks;i++)
    {
      block[i].x_loc=UNPLACED_LOC;
      block[i].y_loc=UNPLACED_LOC;
      block[i].slot_loc=NO_SLOT;
    }

}

int mesh_position_type(int x, int y)
{
  return(mesh[position(x,y)].type);
}

int mesh_used_slots(int x, int y)
{
  return(mesh[position(x,y)].used_slots);
}

/* probably only to be used by unplace */
void remove_block(int whichblock, int x, int y)
{

  int which_block_moved;
  int slot;

  /* consistency check */
  if (whichblock!=mesh[position(x,y)].slots[block[whichblock].slot_loc])
    {
      fprintf(stderr,"remove_block: consistency error with block %s (%d).",block[whichblock].name,whichblock);
      fprintf(stderr,"\t asked to remove from (%d,%d):%d, but found block %d\n",block[whichblock].x_loc,block[whichblock].y_loc,block[whichblock].slot_loc,mesh[position(x,y)].slots[block[whichblock].slot_loc]);
      exit(4);
    }

  if (block[whichblock].slot_loc==(mesh[position(x,y)].used_slots-1))
    {
      /* in last slot, so just shorten block */
      /* which happens below */
    }
  else
    {
      slot=block[whichblock].slot_loc;
      which_block_moved=mesh[position(x,y)].slots[mesh[position(x,y)].used_slots-1];
      mesh[position(x,y)].slots[slot]=mesh[position(x,y)].slots[mesh[position(x,y)].used_slots-1];
      mesh[position(x,y)].timesteps[slot]=mesh[position(x,y)].timesteps[mesh[position(x,y)].used_slots-1];
      block[whichblock].slot_loc=NO_SLOT;
      block[which_block_moved].slot_loc=slot;
    }
  /* both cases */
  mesh[position(x,y)].slots[mesh[position(x,y)].used_slots-1]=EMPTY_SLOT;
  mesh[position(x,y)].timesteps[mesh[position(x,y)].used_slots-1]=UNASSIGNED_TIMESTEP;
  mesh[position(x,y)].used_slots--; 
  

}

/* probably only to be used by place */
void insert_block(int whichblock,  int x, int y, int timestep)

{

  int i;


  if (mesh[position(x,y)].used_slots==mesh[position(x,y)].allocated_slots)
    {
      int *oldslots;
      int *oldtimesteps;
      int oldlength;

      /* allocate more */
      oldslots=mesh[position(x,y)].slots;
      oldtimesteps=mesh[position(x,y)].timesteps;
      oldlength=mesh[position(x,y)].allocated_slots;

      mesh[position(x,y)].allocated_slots++; 
      mesh[position(x,y)].slots=(int *)malloc(mesh[position(x,y)].allocated_slots*sizeof(int));
      mesh[position(x,y)].timesteps=(int *)malloc(mesh[position(x,y)].allocated_slots*sizeof(int));
      
      for(i=0;i<mesh[position(x,y)].allocated_slots;i++)
	{
	  if (i<oldlength)
	    {
	      mesh[position(x,y)].slots[i]=oldslots[i];
	      mesh[position(x,y)].timesteps[i]=oldtimesteps[i];
	    }
	  else
	    {
	      mesh[position(x,y)].slots[i]=EMPTY_SLOT;
	      mesh[position(x,y)].timesteps[i]=UNASSIGNED_TIMESTEP;
	    }
	}
      free(oldslots);
      free(oldtimesteps);
    }
  
  mesh[position(x,y)].slots[mesh[position(x,y)].used_slots]=whichblock;
  mesh[position(x,y)].timesteps[mesh[position(x,y)].used_slots]=timestep;
  block[whichblock].slot_loc=mesh[position(x,y)].used_slots;
  mesh[position(x,y)].used_slots++;


}

void unplace_block(int whichblock)
{


  /* check given valid block id */
  if ((whichblock>num_blocks) || (whichblock<0))
    {
      fprintf(stderr,"unplace_block receives invalid block reference %d, out of bounds 0..%d\n", 
	      whichblock,num_blocks-1);
      exit(3);
    }


  /* check if block currently unplaced */
  if ((block[whichblock].x_loc==UNPLACED_LOC)
      ||(block[whichblock].y_loc==UNPLACED_LOC))
    {
      fprintf(stderr,"unplaced_block receives block which is not placed.");
      // may want to make this fatal
    }
  
  /* remove from position */
  remove_block(whichblock,block[whichblock].x_loc,block[whichblock].y_loc);

  /* mark block as unplaced */
  block[whichblock].x_loc=UNPLACED_LOC;
  block[whichblock].y_loc=UNPLACED_LOC;

  return;
  
}

void place_block(int whichblock, int x, int y, int timestep)
{


  if ((whichblock>num_blocks) || (whichblock<0))
    {
      fprintf(stderr,"place_block receives invalid block reference %d, out of bounds 0..%d\n", 
	      whichblock,num_blocks-1);
      exit(3);
    }


  /* check if block currently unplaced */
  if ((block[whichblock].x_loc!=UNPLACED_LOC)
      ||(block[whichblock].y_loc!=UNPLACED_LOC))
    {
      fprintf(stderr,"placed_block receives block which is already placed.");
      fprintf(stderr,"\tfor proper bookkeepping, unplace block from old location before placing into new.\n");
      exit(4);
    }

  /* check in valid x,y range */
  if ((x>=mesh_width) || (y>=mesh_height))
    {
      fprintf(stderr,"place_block: given an out of range x,y pair.\n");
      fprintf(stderr,"\tgiven (%d,%d), but valid range is (0..%d,0..%d)\n",
	      x,y,mesh_width-1,mesh_height-1);
    }

  /* check valid mesh position for this type */
  if ((block[whichblock].type==INPAD)||(block[whichblock].type==OUTPAD))
    if (mesh[position(x,y)].type!=IOSLOT)
      {
	fprintf(stderr,"place_block: IO %s must be placed in an IOSLOT.\n",
		block[whichblock].name);
	if (mesh[position(x,y)].type==INVALIDSLOT)
	  fprintf(stderr,"\tAttempt to place it in an INVALID (corner) slot (%d,%d)\n",x,y);
	else if (mesh[position(x,y)].type==LUTFF_SLOT)
	  fprintf(stderr,"\tAttempt to place it in a LUT/FF slot (%d,%d)\n",x,y);
	else /* must be LUTFF slut */
	  fprintf(stderr,"\tAttempt to place it in a slot (%d,%d) of type %d\n",
		  x,y,mesh[position(x,y)].type);
	exit(5);
	
      }

  if ((block[whichblock].type==LUT)||
      (block[whichblock].type==LATCH)||
      (block[whichblock].type==LUT_AND_LATCH))
    if (mesh[position(x,y)].type!=LUTFF_SLOT)
      {
	fprintf(stderr,"place_block: LUT and/or LATCH %s (type %d) must be placed in a LUTSLOT.\n",block[whichblock].name,block[whichblock].type);
	if (mesh[position(x,y)].type==INVALIDSLOT)
	  fprintf(stderr,"\tAttempt to place it in an INVALID (corner) slot (%d,%d)\n",x,y);
	else if (mesh[position(x,y)].type==IOSLOT)
	  fprintf(stderr,"\tAttempt to place it in an IO slot (%d,%d)\n",x,y);
	else /* must be IO slut */
	  fprintf(stderr,"\tAttempt to place it in an slot (%d,%d) of type %d\n",
		  x,y,mesh[position(x,y)].type);
	exit(5);
	
      }

  /* place into mesh position */
  insert_block(whichblock,x,y,timestep);

  /* mark block as placed here */
  block[whichblock].x_loc=x;
  block[whichblock].y_loc=y;


}

int timestep(int whichblock)
{
  int x, y, slot;
  int ip, low, high;
  x=block[whichblock].x_loc;
  y=block[whichblock].y_loc;
  slot=block[whichblock].slot_loc;
  if ((x==UNPLACED_LOC) || (y==UNPLACED_LOC) || (slot==NO_SLOT))
    return(-1);  // not placed, this is less than any legitimate placement timeslot

  return(mesh[position(x,y)].timesteps[slot]);

}

void move_block(int whichblock, int dst_x, int dst_y)
{

  int tstep=timestep(whichblock);
  unplace_block(whichblock);
  place_block(whichblock,dst_x,dst_y,tstep);

}

/* added 1/11 */
void update_timestep(int whichblock, int timestep)
{

  int x, y, slot;

  if ((whichblock>num_blocks) || (whichblock<0))
    {
      fprintf(stderr,"update_timestep receives invalid block reference %d, out of bounds 0..%d\n", 
	      whichblock,num_blocks-1);
      exit(3);
    }

  x=block[whichblock].x_loc;
  y=block[whichblock].x_loc;
  slot=block[whichblock].slot_loc;

  mesh[position(x,y)].timesteps[slot]=timestep;
  
}

boolean timestep_in_use(int x, int y, int timestep)
{
  int i;

  for(i=0;i<mesh[position(x,y)].used_slots;i++)
    if (mesh[position(x,y)].timesteps[i]==timestep)
      return(TRUE);

  return(FALSE);

}

boolean produced_in_position(int x, int y, int net)
{

  int i;

  for(i=0;i<mesh[position(x,y)].used_slots;i++)
    if (net==block[mesh[position(x,y)].slots[i]].nets[0])
      return(TRUE);

  return(FALSE);

}

int cluster_io(int x, int y,
	       int luts_per_cluster, 
	       int inputs_per_lut)
{
  int *sort_index;
  float *sort_key;  /* only reason using float is to reuse the existing 
                       heapsort routine */
  int i, j;
  int nelms;
  int unique_inputs;


  sort_index=(int *)malloc(luts_per_cluster*inputs_per_lut*sizeof(int));
  sort_key=(float *)malloc(luts_per_cluster*inputs_per_lut*sizeof(float));
  nelms=0;
  
  for (i=0;i<mesh[position(x,y)].used_slots;i++)
    {

      /* output is position 0; clock is position inputs_per_lut+1 */
      for (j=1;j<=inputs_per_lut;j++)
      {
	if (block[mesh[position(x,y)].slots[i]].nets[j]==OPEN)
	  {
	    /* skip */
	  }
	else
	  {
	    sort_key[nelms]=(float)block[mesh[position(x,y)].slots[i]].nets[j];
	    nelms++;
	  }
	
      }
    }

  my_heapsort(sort_index,sort_key,nelms);

  if (nelms==0)
    return(0);


  /* deal with first input 
     -- always unique, but don't count if produced in this block */
  if (produced_in_position(x,y,(int)sort_key[sort_index[0]])==TRUE)

    {
      unique_inputs=0;

    }
  else
    {
      unique_inputs=1;
    }
    
  /* deal with rest */
  for(i=1;i<nelms;i++)
    {
      /* if we already saw it, don't count it */
      if (sort_key[sort_index[i]]!=sort_key[sort_index[i-1]])
	{
	  if (produced_in_position(x,y,(int)sort_key[sort_index[i]])==FALSE)
	    {
	      unique_inputs++;

	    }
	  else
	    {

	    }
	}
      else
	{

	}
    }


  free(sort_index);
  free(sort_key);
  return(unique_inputs);
  
}


int count_placement_violations(int luts_per_cluster, 
			       int ios_per_iocluster,
			       int inputs_per_lut,
			       int inputs_per_cluster,
			       int report_violations)
{

  int i;
  int j;
  int violations;
  int cluster_size;

  violations=0;
  
  for (i=0;i<mesh_width;i++)
    for (j=0;j<mesh_height;j++)
      {
	cluster_size=0;
	if (mesh[position(i,j)].type==IOSLOT)
	  {
	    cluster_size=ios_per_iocluster;
	  }
	else if (mesh[position(i,j)].type==LUTFF_SLOT)
	  {
	    cluster_size=luts_per_cluster;
	  }
	if (cluster_size>0)
	  {
	    if (mesh[position(i,j)].used_slots>cluster_size)
	      {
		violations+=(mesh[position(i,j)].used_slots-cluster_size);
		if (report_violations==REPORT_VIOLATIONS_LOCATION_COUNT)
		  {
		    fprintf(stderr,"Overused Mesh Position (%d,%d) assigned %d ",i,j,mesh[position(i,j)].used_slots);
		    if (mesh[position(i,j)].type==LUTFF_SLOT)
		      fprintf(stderr,"LUT/FFs");
		    else // assume IO
		      fprintf(stderr,"IOs");
		    fprintf(stderr,".\n");
		  }
		/* for more serious debug, you may want to add code to print out the contents of the overused cluster;
		    ...and you might want to expand to a different
		    report_violations option (e.g. REPORT_VIOLATIONS_FULL_DUMP)
		*/
		
	      }
						 
	    else
	      if (mesh[position(i,j)].type==LUTFF_SLOT)
		{
		  int io=cluster_io(i,j,luts_per_cluster,inputs_per_lut);
		  if (io>inputs_per_cluster)
		    {
		      violations++;
		      if (report_violations==REPORT_VIOLATIONS_LOCATION_COUNT)
			{
			  fprintf(stderr,"I/O exceeded at Mesh Position (%d,%d)\n",
				  i,j);
			  fprintf(stderr,"\tIO=%d > allowed IO of %d \n",
				  io,inputs_per_cluster);
			}
		      
		      
		    }
		}
	  }

      }

  return(violations);

}


int count_unplaced(int report_unplaced)
{

  int unplaced;
  int i;
  unplaced=0;
  for (i=0;i<num_blocks;i++)
    {
      if ((block[i].x_loc==UNPLACED_LOC)
	  ||(block[i].y_loc==UNPLACED_LOC)
	  ||(block[i].slot_loc==NO_SLOT))
	{
	  unplaced++;
	  
	  if (report_unplaced==REPORT_UNPLACED_NAME_TYPE_LOC)
	    {
	      fprintf(stderr,"Unplaced block %s of type %d (%d,%d):%d.\n",
		      block[i].name,block[i].type,
		      block[i].x_loc,
		      block[i].y_loc,
		      block[i].slot_loc);
	    }
	}
    }
  return(unplaced);

}

int print_mesh(int print_type)
{

  int i;
  int j;

  for (j=0;j<mesh_height;j++)
    {
      printf("\n%3d: ",j);
      {
	for (i=0;i<mesh_width;i++)
	  {
	    if (print_type==PRINT_SLOT_TYPES)
	      if (mesh[position(i,j)].type==IOSLOT)
		{
		  printf("I");
		}
	      else if (mesh[position(i,j)].type==LUTFF_SLOT)
		{
		  printf("L");
		}
	      else if (mesh[position(i,j)].type==INVALIDSLOT)
		{
		  printf("C");
		}
	      else
		printf("%d",mesh[position(i,j)].type);

	    if (print_type==PRINT_SLOT_USAGE)
	      printf("[%d]",mesh[position(i,j)].used_slots);

	  }
	
	}
    }

  printf("\n");
	      

}


void move_slot(int x1,int y1, int s1, int x2, int y2)
{

  int l1;
  l1=-1;

  if ((x1>=0) && (x1<mesh_width) && (y1>=0) && (y1<mesh_height))
    if (s1<mesh[position(x1,y1)].used_slots)
      l1=mesh[position(x1,y1)].slots[s1];

  if (l1!=-1)
    {
      move_block(l1,x2,y2);
    }

}



void print_clusters(char *net_file, int cluster_size,
		    int inputs_per_cluster, int clocks_per_cluster, 
		    int lut_size, boolean global_clocks, 
		    boolean muxes_to_cluster_output_pins, boolean *is_clock)
{

  // code from cluster.c:alloc_and_load_cluster_info 


/* Allocates and loads the cluster_contents and cluster_occupancy    *
 * arrays.  Then calls actual print routine. */

 int **cluster_contents;  /* [0..num_clusters-1][0..cluster_size-1] */
 int *cluster_occupancy;  /* [0..num_clusters-1] */
 int icluster;
 int i, j;

 int num_clusters;

 num_clusters=0;
 /* pass over counting non-empty clusters */
  for (j=0;j<mesh_height;j++)
    for (i=0;i<mesh_width;i++)
      if (mesh[position(i,j)].type==LUTFF_SLOT)
	if (mesh[position(i,j)].used_slots>0)
	  num_clusters++;


  cluster_contents = (int **) alloc_matrix (0,num_clusters-1,0,cluster_size-1,
                                 sizeof(int));
  cluster_occupancy = (int *) my_calloc (num_clusters, sizeof (int));

  icluster=0;
  for (j=0;j<mesh_height;j++)
    for (i=0;i<mesh_width;i++)
      if (mesh[position(i,j)].type==LUTFF_SLOT)
	if (mesh[position(i,j)].used_slots>0)
	  {
	    int islot;
	    cluster_occupancy[icluster]=mesh[position(i,j)].used_slots;
	    for (islot=0;islot<cluster_occupancy[icluster];islot++)
	      {
		cluster_contents[icluster][islot]=mesh[position(i,j)].slots[islot];
	      }
	    icluster++;
	  }

  output_clustering (cluster_contents, cluster_occupancy, 
		     cluster_size, inputs_per_cluster, clocks_per_cluster, 
		     num_clusters, lut_size, global_clocks,  
		     muxes_to_cluster_output_pins, 
		     is_clock, net_file);

}

void print_place(char *place_file, char *net_file, char *arch_file)
{

/* Prints out the placement of the circuit.  The architecture and    *
 * netlist files used to generate this placement are recorded in the *
 * file to avoid loading a placement with the wrong support files    *
 * later.                                                            */

 FILE *fp; 
 int ix;
 int iy;
 int islot;

 fp = my_fopen(place_file,"w",0);

 fprintf(fp,"Netlist file: %s   Architecture file: %s\n", net_file,
    arch_file);
 fprintf (fp, "Array size: %d x %d logic blocks\n\n",(mesh_width-2),(mesh_height-2));
 fprintf (fp, "#block name\tx\ty\tsubblk\ttimestep\tblock number\n");
 fprintf (fp, "#----------\t--\t--\t------\t-------\t------------\n");



 for (ix=0;ix<mesh_width;ix++)
   for (iy=0;iy<mesh_height;iy++)
      {
			    
	int used_slots=mesh[position(ix,iy)].used_slots;
	if (used_slots>0)
	  {
	    for (islot=0;islot<used_slots;islot++)
	      {
		int whichblock=mesh[position(ix,iy)].slots[islot];
		fprintf (fp,"%s\t", block[whichblock].name);
		if (strlen(block[whichblock].name) < 8) 
		  fprintf (fp, "\t");

		fprintf (fp,"%d\t%d", ix,iy);
		fprintf(fp,"\t%d", islot);
		fprintf(fp,"\t#timestep=%d ",mesh[position(ix,iy)].timesteps[islot]);
		fprintf (fp, "\tblocknum=%d\n", whichblock);
	      }
	  }
      }
 
 fclose(fp);
}


int find_max_timestep()
{
 int ix;
 int iy;
 int islot;
 int max_timestep=0;

 for (ix=0;ix<mesh_width;ix++)
   for (iy=0;iy<mesh_height;iy++)
     {
       int used_slots=mesh[position(ix,iy)].used_slots;
       if (used_slots>0)
	 {
	   for (islot=0;islot<used_slots;islot++)
	     {
		   
	       max_timestep=max(mesh[position(ix,iy)].timesteps[islot],max_timestep);
	     }
	 }
     }
 return(max_timestep);

}

void print_by_timestep(char *timestep_file_name)
{
 FILE *fp; 
 int ix;
 int iy;
 int islot;
 int itstep;
 int max_timestep;

 fp = my_fopen(timestep_file_name,"w",0);

 // find max timestep
 max_timestep=find_max_timestep();

 for (itstep=0;itstep<=max_timestep;itstep++)
   {
     fprintf(fp,"\nTimestep=%d:\n",itstep);
     for (ix=0;ix<mesh_width;ix++)
       for (iy=0;iy<mesh_height;iy++)
	 {
	   int used_slots=mesh[position(ix,iy)].used_slots;
	   if (used_slots>0)
	     {
	       for (islot=0;islot<used_slots;islot++)
		 {
		   
		   // if matches current timestep
		   if (itstep==mesh[position(ix,iy)].timesteps[islot])
		     {
		       int whichblock=mesh[position(ix,iy)].slots[islot];
		       //   print block and position
		       fprintf (fp,"(%d,%d):%d %s\t", ix,iy,islot,block[whichblock].name);
		     }
		 }
	     }
	 }
   }

	
 fclose(fp);
  

}
