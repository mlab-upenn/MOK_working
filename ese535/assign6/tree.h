#define GLOBAL_CLOCK_LOC -6
#define UNPLACED_LOC -2
#define NO_SLOT -3
#define EMPTY_SLOT -4
#define UNASSIGNED_TIMESTEP -4

#define REPORT_VIOLATIONS_NONE 0
#define REPORT_VIOLATIONS_LOCATION_COUNT 1

#define REPORT_UNPLACED_NONE 0
#define REPORT_UNPLACED_NAME_TYPE_LOC 1

#define PRINT_SLOT_TYPES 1
#define PRINT_SLOT_USAGE 2

#define LEFT 0
#define RIGHT 1

#define UNASSIGNED_NODE -1

#define NODE_TYPE_ERROR 0
#define NODE_TYPE_PE 1
#define NODE_TYPE_SWITCH 2

extern struct tree_node *tree_location;

struct tree_node {
  /* both PEs and switches */
  int node_type;
  int location; /* address of node in tree */
  int height; /* height of node for switch ; 0 for PE */
  int parents; /* number of parents in tree */
  int *parent; /* identify switch(es) this connect to */
  int parent_side; /* left or right*/
  int **parent_in_domains;
  int **parent_out_domains;

  /* PEs only */
  int allocated_slots; /* may have allocated more than in use */
  int used_slots; /* 0..used_slots-1 should be used */
  int *slots; /* pointers to block assigned to this position  */
  int *domain; /* timestep for evaluation of block placed in associated  slot; 
                           this is when it enters the net (or feeds back to
                           self)*/

  /* switches only */
  int left; /* switch or node this connects to */
  int right; /* switch or node this connects to */
  int *left_in_domains;
  int *left_out_domains;
  int *right_in_domains;
  int *right_out_domains;
};


/* allocate physical structure */
void allocate_tree(int* growth_schedule,
		   /* growth_schedule is an array giving the number of parents to each
		      switch at each level;
		      That means, a height N tree will have N+1 elements in the array.
		      The 0th element is the number of parents for the PE.
		      It's possible these can be >2.
		      The rest are for switch nodes.
		      For our planned uses, these will only be 1  (T-switch) or 2 (pi-switch).
		   */   
		   int growth_schedule_length, /* length of growth_schedule array */
		   int domains, /* number of tree time steps */
		   int luts_per_cluster,  /* limit on blocks placed in PE */
		   int ios_per_iocluster /* potentially use to limit IOs placed in a PE */
		   );



/* how big of a tree was created? (# of PEs)*/
int get_size();

/* how many nodes? (including switches) */
int get_nodes();


/* cleanly remove tree and unplace net in case need to rebuild;
   asap_schedule() is first example of use */
void free_tree();


/* number of slots in use */
int tree_used_slots(int loc);


/* use to place the first time */
void place_block(int whichblock, int loc, int timestep);

/* timestep on which block is evaluated or -1 if not assigned */
int timestep(int whichblock);

/* use to move if you know where it should go
   (includes place and unplace)
   does not change timestep (see following) */
void move_block(int whichblock, int dst_loc);

/* use to revise the timestep in place */
void update_timestep(int whichblock, int timestep); 

/* use to unplace if you don't want to use move_block;
   maybe best to avoid */
void unplace_block(int whichblock);

/* is the timestep available at this PE location? */
boolean timestep_in_use(int loc, int timestep);

/* 
   final sanity check that meets targets
*/
int count_placement_violations(int luts_per_cluster, 
			       int ios_per_iocluster,
			       int inputs_per_lut,
			       int inputs_per_cluster,
			       int report_violations);


/* check that everything is given a place. */
int count_unplaced(int report_unplaced);

/* map to find largest timestep. 
   used for checking success and for printing. */
int find_max_timestep();

/* print out tree structure/contents; useful for debug. */
int print_tree();


/* These two print routines are retained from the original VPR usage and do not capture scheduling information */

/* write out clusters (.net file) -- does not capture timesteps */
void print_clusters(char *net_file, int cluster_size,
		    int inputs_per_cluster, 
		    int clocks_per_cluster, 
		    int lut_size, 
		    boolean global_clocks, 
		    boolean muxes_to_cluster_output_pins, 
		    boolean *is_clock);

/* write out placement (VPR format) -- timesteps appear after comment character at end of line*/
void print_place(char *place, char *net, char *arch);


/* The following print routine is specifically for our scheduling case */

/* write out a file arranged by timestep */
void print_by_timestep(char *timestep_file_name);

/* is this node in the path to the specified PE? */
boolean path_to_leaf_side(int tree_loc, int pe_address);

/* dump out nets assigned to each parent domain */
void dump_parent_domains(char *filename);

/* use for inserting blocks into switch to capture blocks in subtree */
void insert_block_switch(int whichblock,  int loc, int timestep);

/* NOTE I HAVE HACKED THIS FUNCTION TO MAKE IT WORK. IT IS NOT VERY SAFE*/
/* THERE IS A POTENTIAL BUG HERE */
void remove_block(int whichblock, int loc);
void insert_block(int whichblock,  int loc, int timestep);