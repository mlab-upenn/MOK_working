/* check if routes are valid */

/* count violations for all routes */
int valid_routes(boolean *is_clock, boolean global_clock, boolean verbose);


/* check a single route */
boolean valid_route(int net, int current_node, int dst, boolean verbose, int limit);

