int which_branch(int max_gain_left, int max_gain_right, int *left, int pmax, int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int capacity, int current_height)
{
  int i;
  int left_count=0;
  int right_count=0;

  for (i=0; i<num_blocks;i++)
  {
    #ifdef DEBUG_PART_PLACE
    printf("Block: %s, i= %d, is free: %d, gain is: %d, is left? %d\n", block[i].name,i, block[i].free, block[i].gain, block[i].left);
    #endif

    if(block[i].loc != GLOBAL_CLOCK_LOC)
    {
          if(block[i].left==1)
          {
            left_count=left_count+1;
          }
          
          if(block[i].left==0)
          {
            right_count=right_count+1;
          }
    }
  }
    

  float balance=0;

  if((right_count+left_count)!=0)
  {
    balance=(float)left_count/(float)(right_count+left_count);
  }

  int num= (left_count+right_count)>>1;

  float r=.5;
  int smax=1;
  
  #ifdef DEBUG_PART_PLACE
  printf("Balance: %f, r %f\n",balance, r);
  printf("Left count %d, Right count %d\n", left_count,right_count);
  printf("Capacity: %d, num %d\n", capacity, num);
  printf("max gain right = %d max gain left =%d\n", max_gain_right, max_gain_left);
  #endif
 
  float lower= floor(r*((float)right_count+(float)left_count)-(float)smax);
  float upper= floor(r*((float)right_count+(float)left_count)+(float)smax);
  //printf("lower %f upper %f\n", lower, upper);



  active_branch=-1;
  int temp_left=0;
  int temp_right=0;

    if(left_count-1>=lower)
    {
      if(max_gain_left>=0 && left_count>0)
      {
        //printf("Pick left branch\n");
        temp_left=1;
      }  
    }
    else if (right_count-1>=lower)
      {
        if(max_gain_right>=0 && right_count>0)
        {
          //printf("Pick right branch\n");
          temp_right=1;
        }
      }

    if(temp_left==1 && temp_right==1)
    {
      if(max_gain_left>= max_gain_right)
      {
        active_branch=0;
      }
      else
      {
        active_branch=1;
      }
    }
    else if(temp_left==1 && temp_right ==0)
    {
      active_branch=0;
    }
    else if(temp_left==0 && temp_right ==1)
    {
      active_branch=1;
    }
    else
    {
      active_branch=-1;
    }
    
    if (current_height== 1)
      
    {
      //printf("current height =1\n");
      //printf("left_count+right count %d\n", left_count+right_count);
      if(left_count+right_count<=capacity && right_count!=0 && max_gain_right>-pmax)
      {
        active_branch=1;
      }
    }
   
    if( active_branch==0)
    {
      if(right_count+1>capacity)
      {
        active_branch=-1;
      }
    }
    if( active_branch==1)
    {
      if(left_count+1>capacity)
      {
        active_branch=-1;
      }
    }
    if(active_branch ==-1)
    {
      if(left_count>capacity)
      {
        active_branch=0;
      }
      if(right_count>capacity)
      {
        active_branch=1;
      }
    }

    //printf("ACTIVE BRANCH IMMEDIATELT POST GUARDS: %d\n", active_branch);
    
 // Determine active block (for debugging purposes)
  if(active_branch==0)
  {
    #ifdef DEBUG_PART_PLACE
    printf("Max Gain: %d, Bucket: %d\n", max_gain_left, (max_gain_left)+pmax);
    printf("Active: %s\n",block[buckets_left[(max_gain_left)+pmax]->cellNumber].name);
    #endif
  }
  else if(active_branch==1)
  {
   #ifdef DEBUG_PART_PLACE
    printf("Max Gain: %d, Bucket: %d\n", max_gain_right, (max_gain_right)+pmax);
    printf("Active: %s\n",block[buckets_right[(max_gain_right)+pmax]->cellNumber].name);
   #endif
  }
  //printf("GOT HERE\n");
  //printf("max gain right %d max gain left %d\n",max_gain_right, max_gain_left);
  if(active_branch==0)
  {
    return buckets_left[max_gain_left+pmax]->cellNumber;
  }
  else if (active_branch==1)
  {
    return buckets_right[max_gain_right+pmax]->cellNumber;
  }
  else
  {
    return active_branch;
  }
  
}

int level_which_branch(int location, int max_gain_left, int max_gain_right, int *left, int pmax, int active_branch, ListPtr *buckets_left, ListPtr *buckets_right, int capacity, int current_height)
{
  int i;
  int left_count=0;
  int right_count=0;
  int active_block_left=-1;
  int active_block_right=-1;
  int active_level=-1;
  int active_block=-1;

  active_level=post_klfm_balance_for_all_levels(location);
  //printf("active level: %d\n",active_level);


  struct listNode *nextCell;

  // If there are any blocks on the left side at all
  // Alternate can occur at PEs
  if(max_gain_left>-pmax)
  {
    int localBlock=-1;
    // Loop through all positive gains
    for(i=max_gain_left; i>-pmax-1; i--)
    {
        //printf("Current Bucket: %d\n",i);
        // Figure out first cell in bucket for gain level
        nextCell=NULL;
        localBlock=buckets_left[i+pmax]->cellNumber;
        //printf("local block: %d\n", localBlock);

        // Make sure that there is something in the bucket
        // If there is find out what the address of the next block is
        if(localBlock>0)
        {
          nextCell=buckets_left[i+pmax]->nextPtr;
        }
        else
        {
          continue;
        }

        // If the current block has a positive gain for the active level
        // Set it as the choice to be moved from the left.
        //printf("Found a block the gain at the active level is %d\n",block[localBlock].level_gain[active_level]);
        if(block[localBlock].level_gain[active_level]>=0)
        {
          active_block_left=buckets_left[i+pmax]->cellNumber;
          break;
        }
        // Else if it is not a good gain and there is another block to try
        // Try the next block, and the next....
        // Until either a good block is found or there are none left
        else if(block[nextCell->cellNumber].name!=NULL)
        {
          do 
          {
            //printf("The next one to check is: %d\n", nextCell->cellNumber);

            if(block[nextCell->cellNumber].level_gain[active_level]>=0)
            {
              //printf("Picked active block %d\n", nextCell->cellNumber);
              active_block_left=nextCell->cellNumber;            }
            else
            {
              //printf("Now gain at the active level is %d\n",block[nextCell->cellNumber].level_gain[active_level]);
              nextCell=nextCell->nextPtr;
            }
            if(active_block_left>=0)
            {
              break;
            }
          }
          while(block[nextCell->cellNumber].name !=NULL);
          
          if(active_block_left>=0)
          {
            break;
          }
        }   
      }
      //printf("max gain left %d cell number %d\n", i, active_block_left);
    }

  // If there are any blocks on the left side at all
  // Alternate can occur at PEs
  if(max_gain_right>-pmax)
  {
    int localBlock=-1;
    // Loop through all positive gains
    for(i=max_gain_right; i>-pmax-1; i--)
    {
        //printf("Current Bucket: %d\n",i);
        // Figure out first cell in bucket for gain level
        nextCell=NULL;
        localBlock=buckets_right[i+pmax]->cellNumber;
        //printf("local block: %d\n", localBlock);

        // Make sure that there is something in the bucket
        // If there is find out what the address of the next block is
        if(localBlock>0)
        {
          nextCell=buckets_right[i+pmax]->nextPtr;
        }
        else
        {
          continue;
        }

        // If the current block has a positive gain for the active level
        // Set it as the choice to be moved from the left.
        //printf("Found a block the gain at the active level is %d\n",block[localBlock].level_gain[active_level]);
        if(block[localBlock].level_gain[active_level]>=0)
        {
          active_block_right=buckets_right[i+pmax]->cellNumber;
          break;
        }
        // Else if it is not a good gain and there is another block to try
        // Try the next block, and the next....
        // Until either a good block is found or there are none left
        else if(block[nextCell->cellNumber].name!=NULL)
        {
          do 
          {
            //printf("The next one to check is: %d\n", nextCell->cellNumber);

            if(block[nextCell->cellNumber].level_gain[active_level]>=0)
            {
              active_block_right=nextCell->cellNumber;
              break;
            }
            else
            {
              //printf("Now gain at the active level is %d\n",block[nextCell->cellNumber].level_gain[active_level]);
              nextCell=nextCell->nextPtr;
            }
            if(active_block_right>=0)
            {
              break;
            } 
          }
          while(block[nextCell->cellNumber].name !=NULL);
        } 
        if(active_block_right>=0)
        {
          break;
        }  

        }
      //printf("max gain right %d cell number %d\n", i, active_block_right);
  }

  //printf("never got here\n");
  for (i=0; i<num_blocks;i++)
  {
    #ifdef DEBUG_PART_PLACE
    printf("Block: %s, i= %d, is free: %d, gain is: %d, is left? %d\n", block[i].name,i, block[i].free, block[i].gain, block[i].left);
    #endif

    if(block[i].loc != GLOBAL_CLOCK_LOC)
    {
          if(block[i].left==1)
          {
            left_count=left_count+1;
          }
          
          if(block[i].left==0)
          {
            right_count=right_count+1;
          }
    }
  }
    

  float balance=0;

  if((right_count+left_count)!=0)
  {
    balance=(float)left_count/(float)(right_count+left_count);
  }

  int num= (left_count+right_count)>>1;

  float r=.5;
  int smax=1;
  
  #ifdef DEBUG_PART_PLACE
  printf("Balance: %f, r %f\n",balance, r);
  printf("Left count %d, Right count %d\n", left_count,right_count);
  printf("Capacity: %d, num %d\n", capacity, num);
  printf("max gain right = %d max gain left =%d\n", max_gain_right, max_gain_left);
  #endif
 
  float lower= floor(r*((float)right_count+(float)left_count)-(float)smax);
  float upper= floor(r*((float)right_count+(float)left_count)+(float)smax);



  active_branch=-1;
  int temp_left=0;
  int temp_right=0;
  //printf("active block left %d, active block right %d active level %d\n", active_block_left, active_block_right, active_level);
      

    if(left_count-1>=lower && active_block_left >=0)
    {
      if(left_count>0  && block[active_block_left].level_gain[active_level]>=0)
      {
        //printf("temp left\n");
        temp_left=1;
      }  
    }
    else if (right_count-1>=lower && active_block_right>=0)
      {
        if(right_count>0 && block[active_block_right].level_gain[active_level]>=0)
        {
          //printf("temp right\n");
          temp_right=1;
        }
      }

    if(temp_left==1 && temp_right==1 && active_block_right>=0 && active_block_left>=0)
    {
      if(block[active_block_right].level_gain[active_level]>=block[active_block_left].level_gain[active_level])
      {
        active_branch=1;
      }
      else
      {
        active_branch=0;
      }
    }
    else if(temp_left==1 && temp_right ==0)
    {
      active_branch=0;
    }
    else if(temp_left==0 && temp_right ==1)
    {
      active_branch=1;
    }
    else
    {
      active_branch=-1;
    }


    
    if( active_branch==0)
    {
      if(right_count+1>capacity)
      {
        active_branch=-1;
      }
    }

    if( active_branch==1)
    {
      if(left_count+1>capacity)
      {
        active_branch=-1;
      }
    }
    if(active_branch ==-1)
    {
      if(left_count>capacity)
      {
        active_branch=0;
      }
      if(right_count>capacity)
      {
        active_branch=1;
      }
    }
 // Determine active block (for debugging purposes)
  if(active_branch==0)
  {
    #ifdef DEBUG_PART_PLACE
    printf("Max Gain: %d, Bucket: %d\n", max_gain_left, (max_gain_left)+pmax);
    printf("Active: %s\n",block[buckets_left[(max_gain_left)+pmax]->cellNumber].name);
    #endif
  }
  else if(active_branch==1)
  {
   #ifdef DEBUG_PART_PLACE
    printf("Max Gain: %d, Bucket: %d\n", max_gain_right, (max_gain_right)+pmax);
    printf("Active: %s\n",block[buckets_right[(max_gain_right)+pmax]->cellNumber].name);
   #endif
  }
  if(active_block_left>=0 && active_branch ==0)
  {
    return active_block_left;
  }
  else if(active_block_right>=0 && active_branch ==1)
  {
    return active_block_right;
  }
  else if(active_branch==0 && active_block_left==-1)
  {
    return buckets_left[max_gain_left+pmax]->cellNumber;
  }
  else if(active_branch==1 && active_block_right==-1)
  {
    return buckets_right[max_gain_right+pmax]->cellNumber;
  }
  else
  {
    return active_branch;
  }
  
  
}