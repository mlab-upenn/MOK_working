/* create a new queue;
   cannot do more than max_size pushes;
   this is not a general queue that works as long as you
   don't have more than max_size things in it.  */
int *new_queue(int max_size);

/* is the queue empty? */
int empty_queue(int *q);

/* remove the head element from the queue and return it */
int pop_queue(int *q);

/* is the specified element in the queue? */
/*  potentially an expensive=O(N) operation */
boolean in_queue(int *q, int val);

/* place the specified element on the tail of the queue */
int push_queue(int *q, int val);

/* release this queue */
void free_queue(int *q);

void reset_queue(int *q);