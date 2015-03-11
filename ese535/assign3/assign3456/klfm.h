int compute_pmax(void);
void initial_partition(int root, int cluster_size);
int initialize_left_branch(boolean *is_clock, int *left, int *right, int root, int *gain, int count, int max_gain_left, struct block_info *blockInfo, int pmax, ListPtr *buckets_left);
int initialize_right_branch(boolean *is_clock, int *left,int *right, int root, int *gain, int count, int max_gain_right, struct block_info *blockInfo, int pmax, ListPtr * buckets_right);
int which_branch(int max_gain_left, int max_gain_right, int *left, int pmax, int active_branch, ListPtr *buckets_left, ListPtr *buckets_right);
void free_blocks(void);
int update_gains_left(int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_left, boolean *left, int current_cut);
int update_gains_right(int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, boolean *left, int current_cut);
int compute_max_gain_left(int pmax, ListPtr *buckets_left);
int compute_max_gain_right(int pmax, ListPtr *buckets_right);
void initialize_F_and_T(boolean *left);


/*
int which_block_to_move(void);
int is_clock(int whichBlock);
int get_number_of_nets(int whichBlock);
void move_block(int whichBlock, int gain);
void update_gains_left(int whichBlock);
void update_gains_right(int whichBlock);*/

