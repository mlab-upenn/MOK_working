int level_update_gains(int active_block,  ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_left, int max_gain_right, int current_cut);
void level_update_case_1(int whichBlock, int active_branch, int whichNet, ListPtr *buckets_left, ListPtr *buckets_right, int pmax,int max_gain_right, int max_gain_left, int level);
void level_update_case_2(int whichBlock, int active_branch, int whichNet, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int max_gain_left, int level);
void level_update_case_move( int active_branch, int whichBlock, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int max_gain_left, int start, int numberOfNets);
void level_update_case_3(int whichBlock, int active_branch, int whichNet, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int max_gain_left, int level);
void level_update_case_4(int whichBlock, int active_branch, int whichNet, ListPtr *buckets_left, ListPtr *buckets_right, int pmax, int max_gain_right, int max_gain_left, int level);
int level_net_max_index(int whichBlock);