#include "list.h"
/* for performing detail routes in assignment 7 and 8 */
/* route all non-local nets */
void detail_route(boolean *is_clock, boolean global_clock, int maxwaves, boolean verbose);

void sort_blocks(boolean *is_clock, boolean global_clock, boolean verbose);

void route_all_nets(boolean *is_clock, boolean global_clock, boolean verbose, int *alap_sort);

void route_net(int whichNet, boolean *is_clock, boolean global_clock, boolean verbose);

void route_pin_pair(int i, int sloc,int loc,boolean verbose);

boolean route_source_sink(int sloc, int loc, int current_node, int previous_node, int src, int dst, int whichNet, int route_num, int local_timestep, int step_num, ListPtr *route_array);

void insert_route(int whichNet, ListPtr *route_array);
void cleanup_reroute(ListPtr *route_array, int whichNet);
void place_in_domain(int location, int route_type, int local_timestep, int whichNet);
//void insert_route(int whichNet, int source, int *route, int route_length, int local_timestep);


