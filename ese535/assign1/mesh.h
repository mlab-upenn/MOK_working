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


extern struct mesh_position *mesh;

enum position_types {BOGUSSLOT, INVALIDSLOT, IOSLOT, LUTFF_SLOT};
/* BOGUS should not be used; created to differentiate assignment versus default value */

struct mesh_position {
  enum position_types type; /* what kind of thing can go here */
  int allocated_slots; /* may have allocated more than in use */
  int used_slots; /* 0..used_slots-1 should be used */
  int *slots; /* pointers to block assigned to this position */
  int *timesteps; /* timestep for evaluation of block placed in associated slot */
};


void allocate_mesh(int x, 
		   int y, 
		   int luts_per_cluster, 
		   int ios_per_iocluster);


/* get mesh width and height since the variables are private to mesh.c *.
int get_mesh_width();
int get_mesh_height();

/* NEW 01/11 */
/* cleanly remove mesh and unplace net in case need to rebuild;
   asap_schedule() is first example of use */
void free_mesh();

/* NEW 01/11 */
/* what type of position is this? */
int mesh_position_type(int x, int y);

/* NEW 01/11 */
/* number of slots in use */
int mesh_used_slots(int x, int y);


/* use to place the first time */
void place_block(int whichblock, int x, int y, int timestep);

/* NEW 01/11 */
/* timestep on which block is evaluated or -1 if not assigned */
int timestep(int whichblock);

/* use to move if you know where it should go
   (includes place and unplace)
   does not change timestep (see following) */
void move_block(int whichblock, int dst_x, int dsty);

/* NEW 01/11 */
/* use to revise the timestep in place */
void update_timestep(int whichblock, int timestep); 

/* use to unplace if you don't want to use move_block;
   maybe best to avoid */
void unplace_block(int whichblock);

/* NEW 01/11 */
/* is the timestep available at this PE location? */
boolean timestep_in_use(int x, int y, int timestep);

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

/* NEW 01/11 */
/* map to find largest timestep. 
   used for checking success and for printing. */
int find_max_timestep();

/* print out mesh structure/contents; useful for debug. */
int print_mesh(int print_type);


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

/* NEW 01/11 */
/* write out a file arranged by timestep */
void print_by_timestep(char *timestep_file_name);

