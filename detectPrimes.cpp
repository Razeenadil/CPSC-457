/*
  Muhammad Razeen Adil 
  CPSC 457 - Assignment 3
  Question 3
*/

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>

//global variables
std::vector<int64_t> numVec;
std::vector<int64_t> result;
std::atomic_bool cancel_flag { false };
std::atomic_bool primeWitness { true };
int64_t element = 0;
pthread_barrier_t barr_id;
int64_t indexFinder = 0;
int64_t interval = 0;


struct Task 
{
  int64_t start, end, id, totalThreads, currElement, currInterval;
  bool isPrime;
};

bool is_prime(int64_t n, int64_t start, int64_t end)
{
  // handle trivial cases
  if (n < 2) return false;
  if (n <= 3) return true; // 2 and 3 are primes
  if (n % 2 == 0) return false; // handle multiples of 2
  if (n % 3 == 0) return false; // handle multiples of 3
  // try to divide n by every number 5 .. sqrt(n)

  //updated start and end point  
  while (start <= end) 
  {
    if (n % start == 0) return false;
    if (n % (start + 2) == 0) return false;
    start += 6;
  }
  return true;
}



void* thread_function(void* a)
{
  //get struct values and initalize variables
  int64_t vecSize = numVec.size();
  struct Task *in = (struct Task*) a;

  while(1)
  {
    //SERIAL WORK
    int res = pthread_barrier_wait(& barr_id);
    if(res == PTHREAD_BARRIER_SERIAL_THREAD)
    {
      while(1)
      {
        if(indexFinder < vecSize)   //still have a number to check
        {
          //get next element
          element = numVec[indexFinder];
          indexFinder++;

          //divide the work
          interval = sqrt(element) / ((in -> totalThreads) * 6);

          //number is small 
          if(element < 1024)
          {
            in -> isPrime = is_prime(element, 5, sqrt(element));
            if(in -> isPrime) result.push_back(element);
            continue;
          }
          else 
          {
            break;
          }
        }
        else    //out of numbers to check
        {
          cancel_flag.store(true, std::memory_order_relaxed);
          break;
        }
      }
      
    }
    pthread_barrier_wait(& barr_id);

    //get struct values     
    in -> currElement = element;
    in -> currInterval = interval;

    if(cancel_flag.load(std::memory_order_relaxed) == true )
    //exit thread out of numbers
    {
      pthread_exit(NULL);
    }
    else 
    {
      //get start and end points
      in -> start = 5 + (in -> id * in -> currInterval * 6);
      in -> end = 5 + ((1 + in -> id) * in -> currInterval * 6);


      if(in -> id == (in -> totalThreads - 1) || in -> totalThreads == 1)  //if this is the last thread we'll have a new end point, or there is one thread so we check the whole number
      {
        in -> end = sqrt(in -> currElement);
      } 

      if(primeWitness.load(std::memory_order_relaxed) == true)    //task cancellation
      {
        in -> isPrime = is_prime(in -> currElement, in -> start, in -> end);
      }

      if(in -> isPrime == false)  //number is not a prime dont check anymore
      {
        primeWitness.store(false, std::memory_order_relaxed);
      }
    }
    

    //SERIAL WORK
    int ress = pthread_barrier_wait(& barr_id);
    if(ress == PTHREAD_BARRIER_SERIAL_THREAD)
    {
      //add number to vector
      if(primeWitness.load(std::memory_order_relaxed) == true && in -> isPrime == true) result.push_back(in -> currElement);
      primeWitness.store(true, std::memory_order_relaxed);
    }
    pthread_barrier_wait(& barr_id);
  }
}


std::vector<int64_t>
detect_primes(const std::vector<int64_t> & nums, int n_threads)
{
  if(nums.empty()) return result;
  
  //initalize variables
  numVec = nums;
  cancel_flag.store(false, std::memory_order_relaxed);
  struct Task t[n_threads];

  //initalzie barrier and thread pool
  pthread_barrier_init(& barr_id, NULL, n_threads);
  pthread_t pool[n_threads];


  for(int64_t i = 0; i < n_threads; i++)
  {
    //initalize struct
    t[i].start = 0;
    t[i].end = 0;
    t[i].id = i;
    t[i].totalThreads = n_threads;
    t[i].isPrime = false; 
    t[i].currElement = 0;
    t[i].currInterval = 0;


    //create threads and error check 
    if(pthread_create(&pool[i], NULL, &thread_function, (void*)&t[i]) != 0)
    {
      printf("ERROR - could not create threads!!\n");
      exit(-1);
    }
  }

  //join threads and collect result 
  for(int64_t i = 0; i < n_threads; i++)
  {
    //join threads and error check 
    if(pthread_join(pool[i], NULL) != 0)
    {
      printf("ERROR - could not join threads!!\n");
      exit(-1);
    }
  }
  pthread_barrier_destroy(&barr_id);
  return result;
}
