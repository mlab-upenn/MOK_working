#include "cost.h"

int *grow_level(int height, int makespan)
{
  int *level_max_io=max_io_by_height_any_level(height,makespan);
  int *result=compute_growth_schedule(height,level_max_io);
  return(result);
}
