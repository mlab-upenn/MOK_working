/* written by andre 1/11 */

#include <stdio.h>
#include <string.h>
#include "util.h"
#include "tree.h"
#include "main.h"
#include "globals.h"


boolean precedence_ok(int whichblock, boolean report_violations)
{

  int low, high, ip;
  boolean result=TRUE;

  int my_timestep=timestep(whichblock);

  if (block[whichblock].type==OUTPAD)
    low=0;
  else 
    low=1;

  if ((block[whichblock].type==LATCH) || (block[whichblock].type==LUT_AND_LATCH))
    high=block[whichblock].num_nets-1; // leave out clock
  else
    high=block[whichblock].num_nets;

  for(ip=low;ip<high;ip++)
    if ((block[net[block[whichblock].nets[ip]].pins[0]].type==LATCH)
	|| (block[net[block[whichblock].nets[ip]].pins[0]].type==LUT_AND_LATCH))
      {
	// Latch is available at start, so always ok
      }
    else
      if (timestep(net[block[whichblock].nets[ip]].pins[0])>=my_timestep)
	{
	  if (report_violations)
	    {
	      fprintf(stderr,"Precedence violation: %s (%d) timestep %d too late for %s (%d) timestep %d\n",
		      block[net[block[whichblock].nets[ip]].pins[0]].name,
		      net[block[whichblock].nets[ip]].pins[0],
		      timestep(net[block[whichblock].nets[ip]].pins[0]),
		      block[whichblock].name,
		      whichblock,
		      my_timestep);
	    }
	  result=FALSE;
	}
  return(result);
}

int check_precedence(boolean report_violations)
{

  int iblock=0;
  int violations=0;
  for (iblock=0;iblock<num_blocks;iblock++)
    {
      if (precedence_ok(iblock,report_violations)==FALSE)
	{
	  violations++;
	}

    }
  return(violations);
}
