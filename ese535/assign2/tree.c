/* written by andre 1/2015 */
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "tree.h"
#include "main.h"
#include "globals.h"
#include "heapsort.h"
#include "output_clustering.h"
#include "domain.h"


struct tree_node *tree_location;
int size;  /* total number of PEs*/
int nodes; /* total numbers of PEs and switches */

void allocate_tree(int *growth_schedule,
                   int growth_schedule_length,
		   int domains,
		   int luts_per_cluster, 
		   int ios_per_iocluster)
{

  int i,j,k;
  int switches;

  if (growth_schedule_length < 1) 
     {
        fprintf(stderr,"allocate_tree: Growth schedule must have more  than 0 entries, got [%d]\n",growth_schedule_length);
        exit(4);
     }
   if  (growth_schedule_length > 30)
     {
        fprintf(stderr,"allocate_tree: not prepared for growth schedule  larger than 30, got [%d]\n",growth_schedule_length);
        exit(4);
     }

  size=1<<(growth_schedule_length);
  switches=0;
  int total_growth=1;
  for (k=1;k<=growth_schedule_length;k++)
      {
       total_growth=total_growth*growth_schedule[k-1];
       int level_switch=total_growth*(size>>k);
       switches+=level_switch;
       //fprintf(stdout,"DEBUG: k=%d, growth %d, total_growth %d, switches %d, total %d\n",k,growth_schedule[k-1],total_growth,level_switch,switches);
      }
  nodes=size+switches;

  tree_location=(struct tree_node *)malloc(nodes*sizeof(struct tree_node)); 

  /* PEs */
  for (i=0;i<size;i++)
      {

	tree_location[i].node_type=NODE_TYPE_PE;
        tree_location[i].height=0;
        tree_location[i].location=i;
	tree_location[i].used_slots=0;
	tree_location[i].slots=(int *)malloc(luts_per_cluster*sizeof(int));
	tree_location[i].domain=(int *)malloc(luts_per_cluster*sizeof(int));
	for (k=0;k<luts_per_cluster;k++)
	  {
	    tree_location[i].slots[k]=EMPTY_SLOT;
	    tree_location[i].domain[k]=DOMAIN_UNUSED;
	  }
	  tree_location[i].allocated_slots=luts_per_cluster;

	  tree_location[i].left_in_domains=(int *)NULL;
	  tree_location[i].left_out_domains=(int *)NULL;
	  tree_location[i].right_in_domains=(int *)NULL;
	  tree_location[i].right_out_domains=(int *)NULL;

	  tree_location[i].parents=growth_schedule[0];
	  tree_location[i].parent=(int *)malloc((sizeof(int))*growth_schedule[0]);
	  tree_location[i].parent_in_domains=(int **)malloc((sizeof(int *))*growth_schedule[0]);
	  tree_location[i].parent_out_domains=(int **)malloc((sizeof(int *))*growth_schedule[0]);
	  for (j=0;j<growth_schedule[0];j++)
	    {
	      tree_location[i].parent[j]=UNASSIGNED_NODE;
	      tree_location[i].parent_in_domains[j]=(int *)malloc((sizeof(int))*domains);
	      tree_location[i].parent_out_domains[j]=(int *)malloc((sizeof(int))*domains);
	      tree_location[i].parent_in_domains[j][DOMAIN_LENGTH]=domains;
	      tree_location[i].parent_out_domains[j][DOMAIN_LENGTH]=domains;
	      for (k=1;k<domains;k++)
		{
		  tree_location[i].parent_in_domains[j][k]=DOMAIN_UNUSED;
		  tree_location[i].parent_out_domains[j][k]=DOMAIN_UNUSED;
		}

	    }
      }

  for (i=size;i<nodes;i++)
    tree_location[i].node_type=NODE_TYPE_ERROR; /* so can check aren't already assigned later */

  fprintf(stdout,"allocated %d total nodes for size %d\n",nodes,size);

  /* switches */
  allocate_subtree(size,0,growth_schedule_length,growth_schedule,total_growth,(int  *)NULL,LEFT,domains);
}

int allocate_subtree(
                       int next_node,
                       int address,
                       int height,
                       int *growth_schedule,
                       int number_switches_at_height,
                       int *parent_switches,
                       int parent_side,
		       int domains
                       )
{
  int i,j,k;
    int current_node;
    int growth;
    int *switches_top_subtree;
    int right_node_start, next_node_start;

    /* growth schedule doesn't go beyond height=1, recognize when no parent */
    if (parent_switches!=(int *)NULL)
      growth=growth_schedule[height];
    else
      growth=0; 

   /* DEBUG print out arguments */
   //fprintf(stdout,"allocate_subtree(%d,%d,%d,...,%d,...,%d)\n",next_node,address,height,number_switches_at_height,parent_side);
   /*
   fprintf(stdout,"\t{");
   if (parent_switches!=(int *)NULL)
     {
     for (i=0;i<number_switches_at_height*growth;i++)
       fprintf(stdout,"%d (l=%d, r=%d) ",parent_switches[i],tree_location[parent_switches[i]].left,tree_location[parent_switches[i]].right);
     }
   fprintf(stdout,"}\n");
   */

    /* DEFENSIVE programming  sanity check*/
   if (parent_switches!=(int *)NULL)
     for (i=0;i<number_switches_at_height*growth;i++)
       if (((parent_side==LEFT) && (tree_location[parent_switches[i]].left!=UNASSIGNED_NODE))||
	   ((parent_side==RIGHT) && (tree_location[parent_switches[i]].right!=UNASSIGNED_NODE)))
	 {
	   fprintf(stderr,"allocate_subtree: ERROR given parents that are  already linked at %d height %d side=%d parent=%d l=%d r=%d\n",address,height,
		   parent_side,parent_switches[i],tree_location[parent_switches[i]].left,tree_location[parent_switches[i]].right);
	 }
	 
   if (height==0) /* at leaf, just link to parents */
     {
       
       if (number_switches_at_height!=1)
	 fprintf(stderr,"allocate_subtree: ERROR at leaf PE %d with number_switches_at_height=%d\n",address,number_switches_at_height);

       for (i=0;i<growth;i++)
           {
              tree_location[address].parent[i]=parent_switches[i];
              tree_location[address].parent_side=parent_side;
              if (parent_side==LEFT)
		tree_location[parent_switches[i]].left=address;
              else if (parent_side==RIGHT)
		tree_location[parent_switches[i]].right=address;
              else
                 fprintf(stderr,"allocate_subtree: bogus parent side %d at  address %d height %d\n",parent_side,address,height);
           }
       return(next_node);
       }

   /* rest for non-PE case, where height>0 */

   switches_top_subtree=(int *)malloc((sizeof(int))*number_switches_at_height);

    /* setup top switches */
    for (i=0;i<number_switches_at_height;i++)
       {
	 current_node=next_node+i;
	 switches_top_subtree[i]=current_node;

	 if (tree_location[current_node].node_type!=NODE_TYPE_ERROR)
	   fprintf(stderr,"allocate_subtree: node %d already assigned type %d, but now reassigning as switch.\n",
		   current_node,tree_location[current_node].node_type);

	 tree_location[current_node].node_type=NODE_TYPE_SWITCH;
	 tree_location[current_node].height=height;
	 tree_location[current_node].location=address;
	 tree_location[current_node].left=UNASSIGNED_NODE;
	 tree_location[current_node].right=UNASSIGNED_NODE;
	 tree_location[current_node].domain=(int *)NULL;
	 tree_location[current_node].slots=(int *)NULL;
	 tree_location[current_node].parents=growth;
	 tree_location[current_node].parent=(int *)malloc((sizeof(int))*growth);
	 tree_location[current_node].left_in_domains=(int *)malloc((sizeof(int))*domains);
	 tree_location[current_node].left_out_domains=(int *)malloc((sizeof(int))*domains);
	 tree_location[current_node].right_in_domains=(int *)malloc((sizeof(int))*domains);
	 tree_location[current_node].right_out_domains=(int *)malloc((sizeof(int))*domains);
	 tree_location[current_node].parent_in_domains=(int **)malloc((sizeof(int *))*growth);
	 tree_location[current_node].parent_out_domains=(int **)malloc((sizeof(int *))*growth);

	 tree_location[current_node].left_in_domains[DOMAIN_LENGTH]=domains;
	 tree_location[current_node].left_out_domains[DOMAIN_LENGTH]=domains;
	 tree_location[current_node].right_in_domains[DOMAIN_LENGTH]=domains;
	 tree_location[current_node].right_out_domains[DOMAIN_LENGTH]=domains;
	 for (k=1;k<domains;k++)
	   {
	     tree_location[current_node].left_in_domains[k]=DOMAIN_UNUSED;
	     tree_location[current_node].left_out_domains[k]=DOMAIN_UNUSED;
	     tree_location[current_node].right_in_domains[k]=DOMAIN_UNUSED;
	     tree_location[current_node].right_out_domains[k]=DOMAIN_UNUSED;
	   }

	 for (j=0;j<growth;j++)
           {
	     if (parent_switches!=(int *)NULL) /* will be null at very top */
	       {
		 
		 if (parent_switches[i]==current_node)
		   fprintf(stderr,"ERROR: node is its own parent %d\n",current_node);

		 tree_location[current_node].parent[j]=parent_switches[i*growth+j];
		 tree_location[current_node].parent_side=parent_side;
		 if (parent_side==LEFT)
		   tree_location[parent_switches[i]].left=current_node;
		 else if (parent_side==RIGHT)
		   tree_location[parent_switches[i]].right=current_node;
		 else
		   fprintf(stderr,"allocate_subtree: bogus parent side %d at  address %d height %d\n",parent_side,address,height);
	       }
	     else
	       {
		 tree_location[current_node].parent[j]=UNASSIGNED_NODE;
	       }
	     tree_location[current_node].parent_in_domains[j]=(int *)malloc((sizeof(int))*domains);
	     tree_location[current_node].parent_out_domains[j]=(int *)malloc((sizeof(int))*domains);
	     tree_location[current_node].parent_in_domains[j][DOMAIN_LENGTH]=domains;
	     tree_location[current_node].parent_out_domains[j][DOMAIN_LENGTH]=domains;
	     for (k=1;k<domains;k++)
	       {
		 tree_location[current_node].parent_in_domains[j][k]=DOMAIN_UNUSED;
		 tree_location[current_node].parent_out_domains[j][k]=DOMAIN_UNUSED;
	       }

	   }
	 
       }
    
    /* recurse left */
    right_node_start=allocate_subtree(next_node+number_switches_at_height,address,height-1,growth_schedule,(number_switches_at_height/growth_schedule[height-1]),switches_top_subtree,LEFT,domains);

    /* recurse right */
    next_node_start=allocate_subtree(right_node_start,(address+(1<<(height-1))),height-1,growth_schedule,(number_switches_at_height/growth_schedule[height-1]),switches_top_subtree,RIGHT,domains);

    free(switches_top_subtree);

    return(next_node_start);
	    
}


int get_size()
{
  return(size);
}
int get_nodes()
{
  return(nodes);
}

void free_tree()
{
  int i,j;
  for (i=0;i<nodes;i++)
     {
	if (tree_location[i].slots!=(int *)NULL)
	  free(tree_location[i].slots);
	if (tree_location[i].domain!=(int *)NULL)
	  free(tree_location[i].domain);
        /* domains */
	if (tree_location[i].left_in_domains!=(int *)NULL)
	  free(tree_location[i].left_in_domains);
	if (tree_location[i].left_out_domains!=(int *)NULL)
	  free(tree_location[i].left_out_domains);
	if (tree_location[i].right_in_domains!=(int *)NULL)
	  free(tree_location[i].right_in_domains);
	if (tree_location[i].right_out_domains!=(int *)NULL)
	  free(tree_location[i].right_out_domains);
	for (j=0;j<tree_location[i].parents;j++)
	  {
	    if (tree_location[i].parent_in_domains!=(int **)NULL)
	      if (tree_location[i].parent_in_domains[j]!=(int *)NULL)
		free(tree_location[i].parent_in_domains[j]);
	    if (tree_location[i].parent_out_domains!=(int **)NULL)
	      if (tree_location[i].parent_out_domains[j]!=(int *)NULL)
		free(tree_location[i].parent_out_domains[j]);
	  }
	if (tree_location[i].parent_in_domains!=(int **)NULL)
	  free(tree_location[i].parent_in_domains);
	if (tree_location[i].parent_out_domains!=(int **)NULL)
	  free(tree_location[i].parent_out_domains);
	if (tree_location[i].parent!=(int *)NULL)
	  free(tree_location[i].parent);
      }
  free(tree_location);
  tree_location=(struct tree_node *)NULL;

  // remove placement information in block since tree has gone away
  for (i=0;i<num_blocks;i++)
    {
      block[i].loc=UNPLACED_LOC;
      block[i].slot_loc=NO_SLOT;
    }

}


int tree_location_type(int loc)
{
  return(tree_location[loc].node_type);
}

 /* TODO */

int tree_used_slots(int loc)
{
  return(tree_location[loc].used_slots);
}

/* probably only to be used by unplace */
void remove_block(int whichblock, int loc)
{

  int which_block_moved;
  int slot;

  /* consistency check */
  if (whichblock!=tree_location[loc].slots[block[whichblock].slot_loc])
    {
      fprintf(stderr,"remove_block: consistency error with block %s (%d).",block[whichblock].name,whichblock);
      fprintf(stderr,"\t asked to remove from (%d):%d, but found block %d\n",block[whichblock].loc,block[whichblock].slot_loc,tree_location[loc].slots[block[whichblock].slot_loc]);
      exit(4);
    }

  if (block[whichblock].slot_loc==(tree_location[loc].used_slots-1))
    {
      /* in last slot, so just shorten block */
      /* which happens below */
    }
  else
    {
      slot=block[whichblock].slot_loc;
      which_block_moved=tree_location[loc].slots[tree_location[loc].used_slots-1];
      tree_location[loc].slots[slot]=tree_location[loc].slots[tree_location[loc].used_slots-1];
      tree_location[loc].domain[slot]=tree_location[loc].domain[tree_location[loc].used_slots-1];
      block[whichblock].slot_loc=NO_SLOT;
      block[which_block_moved].slot_loc=slot;
    }
  /* both cases */
  tree_location[loc].slots[tree_location[loc].used_slots-1]=EMPTY_SLOT;
  tree_location[loc].domain[tree_location[loc].used_slots-1]=UNASSIGNED_TIMESTEP;
  tree_location[loc].used_slots--; 

}

/* probably only to be used by place */
void insert_block(int whichblock,  int loc, int timestep)

{

  int i;


  if (tree_location[loc].used_slots==tree_location[loc].allocated_slots)
    {
      int *oldslots;
      int *oldtimesteps;
      int oldlength;

      /* allocate more */
      oldslots=tree_location[loc].slots;
      oldtimesteps=tree_location[loc].domain;
      oldlength=tree_location[loc].allocated_slots;

      tree_location[loc].allocated_slots++; 
      tree_location[loc].slots=(int *)malloc(tree_location[loc].allocated_slots*sizeof(int));
      tree_location[loc].domain=(int *)malloc(tree_location[loc].allocated_slots*sizeof(int));
      
      for(i=0;i<tree_location[loc].allocated_slots;i++)
	{
	  if (i<oldlength)
	    {
	      tree_location[loc].slots[i]=oldslots[i];
	      tree_location[loc].domain[i]=oldtimesteps[i];
	    }
	  else
	    {
	      tree_location[loc].slots[i]=EMPTY_SLOT;
	      tree_location[loc].domain[i]=UNASSIGNED_TIMESTEP;
	    }
	}
      free(oldslots);
      free(oldtimesteps);
    }
  
  tree_location[loc].slots[tree_location[loc].used_slots]=whichblock;
  tree_location[loc].domain[tree_location[loc].used_slots]=timestep;
  block[whichblock].slot_loc=tree_location[loc].used_slots;
  tree_location[loc].used_slots++;



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
  if ((block[whichblock].loc==UNPLACED_LOC)
      ||(block[whichblock].loc==UNPLACED_LOC))
    {
      fprintf(stderr,"unplaced_block receives block which is not placed.");
      // may want to make this fatal
    }
  
  /* remove from position */
  remove_block(whichblock,block[whichblock].loc);

  /* mark block as unplaced */
  block[whichblock].loc=UNPLACED_LOC;

  return;
  
}

void place_block(int whichblock, int loc, int timestep)
{


  if ((whichblock>num_blocks) || (whichblock<0))
    {
      fprintf(stderr,"place_block receives invalid block reference %d, out of bounds 0..%d\n", 
	      whichblock,num_blocks-1);
      exit(3);
    }


  /* check if block currently unplaced */
  if (block[whichblock].loc!=UNPLACED_LOC)
    {
      fprintf(stderr,"placed_block receives block which is already placed.");
      fprintf(stderr,"\tfor proper bookkeepping, unplace block from old location before placing into new.\n");
      exit(4);
    }

  /* check in valid range */
  if (loc>=size)
    {
      fprintf(stderr,"place_block: given an out of range location.\n");
      fprintf(stderr,"\tgiven (%d), but valid range is (0..%d)\n",
	      loc,size-1);
    }

    if (tree_location[loc].node_type!=NODE_TYPE_PE)
      {
	fprintf(stderr,"place_block: Trying to place %s (type %d) into a non-PE node %d.\n",block[whichblock].name,block[whichblock].type,loc);
	exit(5);
      }

  /* place into tree */
  insert_block(whichblock,loc,timestep);

  /* mark block as placed here */
  block[whichblock].loc=loc;

}

int timestep(int whichblock)
{
  int loc, slot;
  int ip, low, high;
  loc=block[whichblock].loc;
  slot=block[whichblock].slot_loc;
  if ((loc==UNPLACED_LOC) || (slot==NO_SLOT))
    return(-1);  // not placed, this is less than any legitimate placement timeslot

  return(tree_location[loc].domain[slot]);

}

void move_block(int whichblock, int dst)
{

  int tstep=timestep(whichblock);
  unplace_block(whichblock);
  place_block(whichblock,dst,tstep);

}


void update_timestep(int whichblock, int timestep)
{

  int loc, slot;

  if ((whichblock>num_blocks) || (whichblock<0))
    {
      fprintf(stderr,"update_timestep receives invalid block reference %d, out of bounds 0..%d\n", 
	      whichblock,num_blocks-1);
      exit(3);
    }

  loc=block[whichblock].loc;
  slot=block[whichblock].slot_loc;

  tree_location[loc].domain[slot]=timestep;
  
}

boolean timestep_in_use(int loc, int timestep)
{
  int i;

  for(i=0;i<tree_location[loc].used_slots;i++)
    if (tree_location[loc].domain[i]==timestep)
      return(TRUE);

  return(FALSE);

}

boolean produced_in_position(int loc, int net)
{

  int i;

  for(i=0;i<tree_location[loc].used_slots;i++)
    if (net==block[tree_location[loc].slots[i]].nets[0])
      return(TRUE);

  return(FALSE);

}

int cluster_io(int loc,
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
  
  for (i=0;i<tree_location[loc].used_slots;i++)
    {

      /* output is position 0; clock is position inputs_per_lut+1 */
      for (j=1;j<=inputs_per_lut;j++)
      {
	if (block[tree_location[loc].slots[i]].nets[j]==OPEN)
	  {
	    /* skip */
	  }
	else
	  {
	    sort_key[nelms]=(float)block[tree_location[loc].slots[i]].nets[j];
	    nelms++;
	  }
	
      }
    }

  my_heapsort(sort_index,sort_key,nelms);

  if (nelms==0)
    return(0);


  /* deal with first input 
     -- always unique, but don't count if produced in this block */
  if (produced_in_position(loc,(int)sort_key[sort_index[0]])==TRUE)

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
	  if (produced_in_position(loc,(int)sort_key[sort_index[i]])==FALSE)
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

  /*  maybe todo -- use ios_per_iocluster as limit on IOs placed in a
  cluster  -- not checking now*/

  for (i=0;i<size;i++)
      {
	cluster_size=0;
	if (cluster_size>0)
	  {
	    if (tree_location[i].used_slots>luts_per_cluster)
	      {
		violations+=(tree_location[i].used_slots-cluster_size);
		if (report_violations==REPORT_VIOLATIONS_LOCATION_COUNT)
		  {
		    fprintf(stderr,"Overused PE (%d) assigned %d ",i,tree_location[i].used_slots);
		    fprintf(stderr,".\n");
		  }
		/* for more serious debug, you may want to add code to print out the contents of the overused cluster;
		    ...and you might want to expand to a different
		    report_violations option (e.g. REPORT_VIOLATIONS_FULL_DUMP)
		*/
		
	      }
						 
	    else
	      if (tree_location[i].node_type==NODE_TYPE_PE)
		{
		  int io=cluster_io(i,luts_per_cluster,inputs_per_lut);
		  if (io>inputs_per_cluster)
		    {
		      violations++;
		      if (report_violations==REPORT_VIOLATIONS_LOCATION_COUNT)
			{
			  fprintf(stderr,"I/O exceeded at Tree Location (%d)\n",
				  i);
			  fprintf(stderr,"\tIO=%d > allowed IO of %d \n", io,inputs_per_cluster);
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
      if ((block[i].loc==UNPLACED_LOC)
	  ||(block[i].slot_loc==NO_SLOT))
	{
	  unplaced++;
	  
	  if (report_unplaced==REPORT_UNPLACED_NAME_TYPE_LOC)
	    {
	      fprintf(stderr,"Unplaced block %s of type %d (%d):%d.\n",
		      block[i].name,block[i].type,
		      block[i].loc,
		      block[i].slot_loc);
	    }
	}
    }
  return(unplaced);

}


void print_node(int i)
{
   int j;
   if (tree_location[i].node_type==NODE_TYPE_PE)
       printf("PE %d\n",tree_location[i].location);
   else if (tree_location[i].node_type==NODE_TYPE_SWITCH)
     printf("Switch %d in subtree %d  at h=%d\n",i,tree_location[i].location,tree_location[i].height);
   else
       printf("Unkown node type at %d, height=%d\n",tree_location[i].location,tree_location[i].height);
   printf("\tparents (%d):",tree_location[i].parents);
   for (j=0;j<tree_location[i].parents;j++)
     printf("%d ",tree_location[i].parent[j]);
   printf("\n");
   if (tree_location[i].node_type==NODE_TYPE_SWITCH)
     {
        printf("\tleft=%d, right=%d\n",tree_location[i].left,tree_location[i].right);
     }
}

int print_tree()
{
  int i;
  for (i=0;i<nodes;i++)
    print_node(i);
}


void move_slot(int loc1,int s1, int loc2)
{

  int l1;
  l1=-1;

  if ((loc1>=0) && (loc1<size) && (loc2>=0) && (loc2<size))
    if (s1<tree_location[loc1].used_slots)
      l1=tree_location[loc1].slots[s1];

  if (l1!=-1)
    {
      move_block(l1,loc2);
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
  for (i=0;i<size;i++)
      if (tree_location[i].node_type==NODE_TYPE_PE)
	if (tree_location[i].used_slots>0)
	  num_clusters++;


  cluster_contents = (int **) alloc_matrix (0,num_clusters-1,0,cluster_size-1,
                                 sizeof(int));
  cluster_occupancy = (int *) my_calloc (num_clusters, sizeof (int));

  icluster=0;
    for (i=0;i<size;i++)
      if (tree_location[i].node_type==NODE_TYPE_PE)
	if (tree_location[i].used_slots>0)
	  {
	    int islot;
	    //printf("DEBUG: cluster %d has %d used slots\n",i,tree_location[i].used_slots);
	    cluster_occupancy[icluster]=tree_location[i].used_slots;
	    for (islot=0;islot<cluster_occupancy[icluster];islot++)
	      {
		cluster_contents[icluster][islot]=tree_location[i].slots[islot];
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
 int iloc;
 int islot;

 fp = my_fopen(place_file,"w",0);

 fprintf(fp,"Netlist file: %s   Architecture file: %s\n", net_file,
    arch_file);
 fprintf (fp, "Tree size: %d logic blocks\n\n",size);
 fprintf (fp, "#block name\tloc\tsubblk\ttimestep\tblock number\n");
 fprintf (fp, "#----------\t--\t------\t-------\t------------\n");



 for (iloc=0;iloc<size;iloc++)
      {
			    
	int used_slots=tree_location[iloc].used_slots;
	if (used_slots>0)
	  {
	    for (islot=0;islot<used_slots;islot++)
	      {
		int whichblock=tree_location[iloc].slots[islot];
		fprintf (fp,"%s\t", block[whichblock].name);
		if (strlen(block[whichblock].name) < 8) 
		  fprintf (fp, "\t");

		fprintf (fp,"%d", iloc);
		fprintf(fp,"\t%d", islot);
		fprintf(fp,"\t#timestep=%d ",tree_location[iloc].domain[islot]);
		fprintf (fp, "\tblocknum=%d\n", whichblock);
	      }
	  }
      }
 
 fclose(fp);
}


int find_max_timestep()
{
 int iloc;
 int islot;
 int max_timestep=0;

 for (iloc=0;iloc<size;iloc++)
     {
       int used_slots=tree_location[iloc].used_slots;
       if (used_slots>0)
	 {
	   for (islot=0;islot<used_slots;islot++)
	     {
	       max_timestep=max(tree_location[iloc].domain[islot],max_timestep);
	     }
	 }
     }
 return(max_timestep);

}

void print_by_timestep(char *timestep_file_name)
{
 FILE *fp; 
 int iloc;
 int islot;
 int itstep;
 int max_timestep;

 fp = my_fopen(timestep_file_name,"w",0);

 // find max timestep
 max_timestep=find_max_timestep();

 for (itstep=0;itstep<=max_timestep;itstep++)
   {
     fprintf(fp,"\nTimestep=%d:\n",itstep);
     for (iloc=0;iloc<size;iloc++)
	 {
	   int used_slots=tree_location[iloc].used_slots;
	   if (used_slots>0)
	     {
	       for (islot=0;islot<used_slots;islot++)
		 {
		   
		   // if matches current timestep
		   if (itstep==tree_location[iloc].domain[islot])
		     {
		       int whichblock=tree_location[iloc].slots[islot];
		       //   print block and position
		       fprintf (fp,"(%d):%d %s\t", iloc,islot,block[whichblock].name);
		     }
		 }
	     }
	 }
   }

 fclose(fp);

}


boolean path_to_leaf_side(int tree_loc, int pe_address)
{

  int node_addr=tree_location[tree_loc].location;
  int node_height=tree_location[tree_loc].height;

  if ((node_addr>>node_height)==(pe_address>>node_height))
    return(TRUE);
  else
    return(FALSE);
  
}

void dump_parent_domains(char *filename)
{

 FILE *fp; 
 int i,j;

 fp = my_fopen(filename,"w",0);

 for(i=0;i<nodes;i++)
   {
     for (j=0;j<tree_location[i].parents;j++)
       {
	 if (tree_location[i].node_type==NODE_TYPE_PE)
	   fprintf(fp,"PE ");
	 else
	   fprintf(fp,"Switch ");
	 fprintf(fp,"%d (height=%d, addr=%d) in%d: ",i,tree_location[i].height,tree_location[i].location,j);
	 print_domain(fp,tree_location[i].parent_in_domains[j]);
	 fprintf(fp,"\n");
	 if (tree_location[i].node_type==NODE_TYPE_PE)
	   fprintf(fp,"PE ");
	 else
	   fprintf(fp,"Switch ");
	 fprintf(fp,"%d (height=%d, addr=%d) out%d: ",i,tree_location[i].height,tree_location[i].location,j);
	 print_domain(fp,tree_location[i].parent_out_domains[j]);
	 fprintf(fp,"\n");
       }
     // ALSO INCLUDE right, left for debugging
     if (tree_location[i].left_in_domains!=(int *)NULL)
       {
	 if (tree_location[i].node_type==NODE_TYPE_PE)
	   fprintf(fp,"PE ");
	 else
	   fprintf(fp,"Switch ");
	 fprintf(fp,"%d (height=%d, addr=%d) left in: ",i,tree_location[i].height,tree_location[i].location);
	 print_domain(fp,tree_location[i].left_in_domains);
	 fprintf(fp,"\n");
	 if (tree_location[i].node_type==NODE_TYPE_PE)
	   fprintf(fp,"PE ");
	 else
	   fprintf(fp,"Switch ");
	 fprintf(fp,"%d (height=%d, addr=%d) left out: ",i,tree_location[i].height,tree_location[i].location);
	 print_domain(fp,tree_location[i].left_out_domains);
	 fprintf(fp,"\n");
       }
     if (tree_location[i].right_in_domains!=(int *)NULL)
       {
	 if (tree_location[i].node_type==NODE_TYPE_PE)
	   fprintf(fp,"PE ");
	 else
	   fprintf(fp,"Switch ");
	 fprintf(fp,"%d (height=%d, addr=%d) right in: ",i,tree_location[i].height,tree_location[i].location);
	 print_domain(fp,tree_location[i].right_in_domains);
	 fprintf(fp,"\n");
	 if (tree_location[i].node_type==NODE_TYPE_PE)
	   fprintf(fp,"PE ");
	 else
	   fprintf(fp,"Switch ");
	 fprintf(fp,"%d (height=%d, addr=%d) right out: ",i,tree_location[i].height,tree_location[i].location);
	 print_domain(fp,tree_location[i].right_out_domains);
	 fprintf(fp,"\n");
       }
   }

 fclose(fp);

}
