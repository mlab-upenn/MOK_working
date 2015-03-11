extern int *level;

#define LEVEL_UNDETERMINED -2

/* minimum scheduling delay with no capacity bounds */
int asap_delay();

/* print out block and level -- mostly used for debugging */
void print_level_assignment();

/* perform an asap schedule */
void asap_schedule(int width, int height, int cluster_size, int input_cluster_size, 
		  int makespan);

