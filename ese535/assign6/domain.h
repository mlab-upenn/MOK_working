/* routines for working with domains...at least for assign2 */

#define DOMAIN_UNUSED -1

/* we'll put the length of the domain in the 0 slot */
#define DOMAIN_LENGTH 0

#define INITIAL_DOMAINS 10


int *unique_add_domain(int net, int *domain);

boolean in_domain(int net, int *domain);

int count_in_domain(int *domain);

/* only count those nets assigned to specified level */
int count_in_domain_by_level(int level, int *domain); 

/* print nets in domain to file */
void print_domain(FILE *fp, int *domain);

