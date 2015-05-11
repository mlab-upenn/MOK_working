/* routine to perform partitioning-based placement */
void part_place(int size, int cluster_size, boolean recurse, boolean level_max, boolean verbose, boolean *is_clock, boolean global_clock);
void calculate_and_write_top_cut(int size, char *cost_file, boolean VERBOSE);

