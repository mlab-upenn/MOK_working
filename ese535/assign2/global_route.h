/* for performing global routes for assign2 (and probably 3-6) */

/* route all non-local nets */
void global_route(boolean verbose);

/* route a single two-point net (one source to one destination) */
void route(int net, int current_node, int src, int dst, boolean verbose);

