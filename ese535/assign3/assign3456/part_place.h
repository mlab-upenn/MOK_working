
struct block_info {
  /* both PEs and switches */
  int block_address;
  int gain;
  char *name;
  boolean left;
  boolean free;
  int loc;
};

/* routine to perform partitioning-based placement */
int part_place(int size, int cluster_size, boolean recurse, boolean level_max, boolean verbose, boolean *is_clock, boolean global_clock);
void calculate_and_write_top_cut(int size, char *cost_file, boolean VERBOSE, int current_cut);
int calculate_cut(boolean *left);


