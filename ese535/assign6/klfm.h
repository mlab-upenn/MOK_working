// Helper function to compute the maximum number of pins on a block
int compute_pmax(void);

// Helper function which frees all the blocks after a KLFM pass
void free_blocks(void);

// Create initial partition for KLFM procedure
void initial_partition(int root, int cluster_size);

// Helper function to compute the intitial values for F_left, T_left, F_right, T_right
void initialize_F_and_T(int *left);

// Compute gains and make buckets for left branch. Returns max gain
int initialize_left_branch(boolean *is_clock, int *left, int *right, int root, int *gain, int max_gain_left, int pmax, ListPtr *buckets_left, boolean rec);

// Compute gains and make buckets for right branch. Returns max gain
int initialize_right_branch(boolean *is_clock, int *left,int *right, int root, int *gain, int max_gain_right, int pmax, ListPtr * buckets_right, boolean rec);

// Determine which branch to perform KLFM pass on
int which_branch(int max_gain_left, int max_gain_right, int *left, int pmax, int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int capacity, int current_height);

// Update gains on the left branch
int update_gains_left(int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_left, int current_cut);

// Update gains on the right branch
int update_gains_right(int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int current_cut);

// Helper function to compute the maximum gain on the left branch after an update
int compute_max_gain_left(int pmax, ListPtr *buckets_left);

// Helper function to compute the maximum gain on the right branch after an update
int compute_max_gain_right(int pmax, ListPtr *buckets_right);

// Rearrange blocks
void rearrange_blocks (int switch_num);

// Recursive function to get blocks into PEs
int rec_KLFM(int current_height, struct tree_node *left_switch, struct tree_node *right_switch, int pmax, boolean *is_clock, int location);

// Repartition children
void repartition(int height,int number_of_blocks, int *sub_left, int *sub_right, int location, int cluster_size, int root);

// Compute F and Ts for a subtree
void sub_F_and_T(int height, int *sub_left);

void recurse_tree(int height, int location, int pmax, boolean *is_clock, int capacity, int cluster_size, int root);

int rec_initialize_left_branch(int *left, int *right, int tree_loc, int max_gain_left, int pmax, ListPtr * buckets_left);
int rec_initialize_right_branch(int *left, int *right, int tree_loc, int max_gain_right, int pmax, ListPtr * buckets_right);
int calculate_gains(int blockLoc);
void check_gains(int blockLoc);
int array_scan(int whichElem, int *array, int num_elem);
int KLFM(int current_height, int pmax, boolean *is_clock, int location, int capacity, int cluster_size, int root);

void print_f_and_t(void);

void enforce_balance(int switch_num, int capacity, int pmax, ListPtr *buckets_left, ListPtr *buckets_right, int current_cut);



/*
int which_block_to_move(void);
int is_clock(int whichBlock);
int get_number_of_nets(int whichBlock);
void move_block(int whichBlock, int gain);
void update_gains_left(int whichBlock);
void update_gains_right(int whichBlock);*/

