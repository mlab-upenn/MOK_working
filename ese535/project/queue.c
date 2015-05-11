#include <stdio.h>
#include "util.h"

#define QUEUE_HEAD 0
#define QUEUE_TAIL 1
#define QUEUE_MAXSIZE 2
#define QUEUE_FIRSTPOS 3

/* not a fully general queue:
   i) cannot do more than max_size pushes
   ii) currently only allows a single entry of each int/value in the queue
*/

int *new_queue(int max_size)
{
  /* assume will never put more than max_size things in queue */
  /* Hackish, but quick and dirty */
  int *queue=(int *)my_malloc(sizeof(int)*(max_size+4));

  queue[QUEUE_HEAD]=QUEUE_FIRSTPOS;
  queue[QUEUE_TAIL]=QUEUE_FIRSTPOS-1;
  queue[QUEUE_MAXSIZE]=max_size;

  return(queue);
}

boolean empty_queue(int *q)
{
  if(q[QUEUE_TAIL]==(q[QUEUE_HEAD]-1))
    return(TRUE);
  else
    return(FALSE);
}

int pop_queue(int *q)
{
  int res;

  if (empty_queue(q))
    {
      fprintf(stderr,"Attempt to pop empty queue.\n");
      exit(31);
    }
  res=q[q[QUEUE_HEAD]];
  int *temp = &q[q[QUEUE_HEAD]];
  q[QUEUE_HEAD]++;
  return(res);
}

boolean in_queue(int *q, int val)
{
  int i;
  for (i=QUEUE_FIRSTPOS;i<=q[QUEUE_TAIL];i++)
    if (q[i]==val)
      {
	return(TRUE);
      }
  return(FALSE);
}

int push_queue(int *q, int val)
{


  /* check didn't grow too big */
  if (q[QUEUE_TAIL]>(q[QUEUE_MAXSIZE]+4))
    {
      fprintf(stderr,"Queue exceeds planned capacity %d.\n",q[QUEUE_TAIL]);
      exit(32);
    }
  if (!in_queue(q,val)) /* shouldn't put a node in the queue twice */
    {
      q[QUEUE_TAIL]++;
      q[q[QUEUE_TAIL]]=val;
      
    }
  else
    {

    }
}

void free_queue(int *q)
{
  free(q);
}

void reset_queue(int *q)
{
  q[QUEUE_HEAD]=QUEUE_FIRSTPOS;
}
