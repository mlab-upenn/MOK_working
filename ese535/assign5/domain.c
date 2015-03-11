/* AMD 1/18/2015 */
/* routines for working with domains...at least for assign2 */
/* These routines use domains in a different way than we will 
   in the later half of the course. */
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "globals.h"
#include "util.h"
#include "asap.h"
#include "domain.h"

int *unique_add_domain(int net, int *domain)
{
  int i;
  int *expanded;

  for (i=1;i<domain[DOMAIN_LENGTH]; i++)
    if (domain[i]==DOMAIN_UNUSED)
      {
	domain[i]=net;
	return(domain);
      }
    else if (domain[i]==net)
      return(domain);

  // ran out of space, need to expand

  expanded=(int *)malloc(sizeof(int)*domain[DOMAIN_LENGTH]*2);
  expanded[DOMAIN_LENGTH]=domain[DOMAIN_LENGTH]*2;
  for (i=1;i<domain[DOMAIN_LENGTH];i++)
    expanded[i]=domain[i];
  // add new net
  expanded[domain[DOMAIN_LENGTH]]=net;
  // initialize
  for (i=domain[DOMAIN_LENGTH]+1;i<expanded[DOMAIN_LENGTH];i++)
    expanded[i]=DOMAIN_UNUSED;

  free(domain); 

  return(expanded);
  

}

int count_in_domain(int *domain)
{
  int res=0;
  for (res=1;res<domain[DOMAIN_LENGTH]; res++)
    if (domain[res]==DOMAIN_UNUSED)  
      return(res-1);
}

/* only count those nets assigned to specified level */
int count_in_domain_by_level(int target_level, int *domain)
{
  int i, res;
  res=0;
  for (i=1;i<domain[DOMAIN_LENGTH]; i++)
    if (domain[i]==DOMAIN_UNUSED)  
      return(res);
    else
      if (level[net[domain[i]].pins[0]]==target_level)
	res++;
  
  return(res);
}


boolean in_domain(int net, int *domain)
{

  int i;

  for (i=1;i<domain[DOMAIN_LENGTH]; i++)
    if (domain[i]==DOMAIN_UNUSED)
      return(FALSE);
    else if (domain[i]==net)
      return(TRUE);

  return(FALSE);

}


void print_domain(FILE *fp, int *domain)
{
  int i=0;
  for (i=1;i<domain[DOMAIN_LENGTH]; i++)
    if (domain[i]==DOMAIN_UNUSED)  
      return;
    else
      fprintf(fp," %s",net[domain[i]].name);
}
