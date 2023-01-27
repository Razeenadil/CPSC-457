/*
  Muhammad Razeen Adil 
  Assignment 3 - Q1 
  CPSC 457 
*/

#include <stdio.h>   
#include <pthread.h>  
#include <string> 
#include <unistd.h>   
#include <cmath>

#include "calcpi.h"

struct Task 
{
  int start_X, end_X, radius;
  uint64_t partial_count;
};

void* thread_function(void *a)
{
  //initalize variables and struct
  struct Task *in = (struct Task*) a;
  int start = in->start_X;
  int end = in->end_X;
  int r = in->radius;
  uint64_t partialCount = in->partial_count;


  //loop for calculating PI
  for(double x = start; x <= end; x++)
  {
    for(double y = 0; y <= r; y++)
    {
      if(x*x + y*y <= double(r)*r) partialCount++;
    }
  }

  in -> partial_count = partialCount; //store updated count
  pthread_exit(NULL);
}

uint64_t count_pixels(int r, int n_threads)
{
  //if there are extra threads given by user dont initialize all of them 
  if(n_threads > r)
  {
    n_threads = r;
  }

  //initalize threads and variables 
  uint64_t count = 0;
  pthread_t pool[n_threads];
  struct Task t[n_threads];

  //set intervals for work 
  int lastend = 1;
  int interval = 0;

  if(r == 0)
  {
    interval = 0;
  }
  else 
  {
    interval = (int) std::round (r / n_threads) - 1;
  }
  

  for(int i = 0; i < n_threads; i++)
  {
    t[i].start_X = lastend;
    t[i].end_X = lastend + interval;
    t[i].radius = r;
    t[i].partial_count = 0;

    if(i == n_threads - 1) t[i].end_X = r;

    lastend = t[i].end_X + 1;

    //create threads and error check 
    if(pthread_create(&pool[i], NULL, thread_function, (void *)&t[i]) != 0)
    {
      exit(-1);
    }
  }

  //join threads
  for(int i = 0; i < n_threads; i++)
  {
    //join threads and error check 
    if(pthread_join(pool[i], NULL) != 0)
    {
      exit(-1);
    }
  }

  //collect results 
  for(int i = 0; i < n_threads; i++)
  {
    count = count + t[i].partial_count;
  }

  return count * 4 + 1;
}
