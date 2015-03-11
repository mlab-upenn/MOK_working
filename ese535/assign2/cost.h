/* routines to compute cost input and costs for assign2 (and probably used for assign 3-6) */


/* max over all subtrees (switches for assign2) at given height */
int *max_io_by_height(int height);

/* only consider nets driven at specified level */
int **max_io_by_height_at_level(int height, int level);


/* max over all levels */
int *max_io_by_height_any_level(int height, int makespan, int **test);

/* identify minimum cost growth schedule to support traffic indicated */
int *compute_growth_schedule(int height, int *max_io_by_height);

/* compute cost given growth schedule */
int cost_of_growth_schedule(int height, int *growth_schedule);

/* perform calculations and print to file */
void calculate_and_print_costs(char *source_file, char *cost_file, int makespan, int height);
