//int *alap_delay(boolean *is_clock);
void alap_delay(void);
int all_succ_sched(int whichBlock, int current_timestep, int *is_clock);
void process_ready_queue(int *ready_queue, int *alap_level, int current_timestep);