/*
    Muhammad Razeen Adil
    Assignment 4 
    Question 2
*/

#include "scheduler.h"
#include "common.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <algorithm>    
#include <utility>
#include <vector> 
#include <limits>
#include <math.h>    

//populates burstsLeft with bursts from processes
void getBursts(const std::vector<Process> & processes,  std::vector<int64_t>  &burstsLeft,  std::vector<int> &jq)
{
    for(auto &p : processes)
    {
        burstsLeft.push_back(p.burst);
        jq.push_back(p.id);
    }   
}

//check if see if any start_time == -1, if so return true otherwise false
bool checkProcessStartTime(const std::vector<int> & rq, const std::vector<Process> & processes)
{
    for(auto & r : rq)
    {
        if(processes[r].start_time == -1) return false;
        
    }
    return true;
}

//finds the min burstsleft from rq and returns the min 
//if number of burst reach 0 return -1 (flag).
int64_t findSmallestBurst(const std::vector<int64_t> & burstsLeft, int64_t quantum, const std::vector<int> & rq)
{
    int64_t min = std::numeric_limits<std::int64_t>::max();
    for(auto & r : rq)
    {
        if(burstsLeft[r] - quantum <= 0) return -1;

        if(burstsLeft[r] <= min) min = burstsLeft[r];

    }
    return min;
}



void simulate_rr(int64_t quantum, int64_t max_seq_len, std::vector<Process> & processes, std::vector<int> & seq) 
{
    seq.clear();
    std::vector<int64_t> burstsLeft;
    std::vector<int> rq, jq;
    int64_t curr_time = 0;
    int cpu = -1;
    
    if(processes.size() == 0) return;

    getBursts(processes, burstsLeft, jq);   //populate bursts

    //curr_time starts at arrival of first process
    curr_time = processes[0].arrival;

    if(curr_time > 0) seq.push_back(cpu);   //cpu is idle till process first process arrives


    while(1)
    {
        //executing last process **HINT
        if(jq.empty() && rq.empty())
        {
            //set start and finish times if not set
            processes[cpu].finish_time = curr_time + burstsLeft[cpu];   
            if(processes[cpu].start_time == -1) processes[cpu].start_time = curr_time;

            //update seq. Check for consecutive duplicates. Check if size is correct
            if((seq.back() != cpu && (int64_t)(seq.size()) < max_seq_len) || seq.empty())
            {
                seq.push_back(cpu);
            }
            return;     //sim is over
        }

        //check if a process is complete
        if(cpu != -1 && burstsLeft[cpu] == 0)
        {
            processes[cpu].finish_time = curr_time; //set finish time
            cpu = -1;
            continue;
        }


        //checks if process arrives
        if(!jq.empty() && processes[jq.front()].arrival == curr_time)
        {
            rq.push_back(jq.front());   //populate rq
            jq.erase(jq.begin());       //remove value from jq
            continue;
        }

        //rq is empty and cpu is idle **HINT
        if(cpu == -1 && rq.empty())
        {
            if(!jq.empty())
            {
                //push time to next arrival cause we have nothing to do, also see the start_time if not set
                curr_time = processes[jq.front()].arrival;
                if(processes[jq.front()].start_time == -1) processes[jq.front()].start_time = curr_time;

                //update seq. Check for consecutive duplicates. Check if size is correct
                if((seq.back() != cpu && (int64_t)(seq.size()) < max_seq_len) || seq.empty())
                {
                    seq.push_back(cpu);
                }
            }
            continue;
        }

        
        //for updating seq
        if(cpu == -1 && !rq.empty())        //rq.front() will be the next job so add it to seq
        {
            if((seq.back() != rq.front() && (int64_t)(seq.size()) < max_seq_len) || seq.empty())
            {
                seq.push_back(rq.front());
            }
        } 
        else if (cpu != -1 || rq.empty())       //job already on cpu or no job atm
        {
            if((seq.back() != rq.front() && (int64_t)(seq.size()) < max_seq_len) || seq.empty())
            {
                seq.push_back(rq.front());
            }
        }
        

        // When cpu is idle, rq is not empty and process still arriving **HINT
        if(cpu == -1 && !rq.empty() && !jq.empty() && (int64_t)seq.size() == max_seq_len)
        {
            //check if every process in rq has a start time 
            if( checkProcessStartTime(rq, processes) == true)
            {
                //math for calculating smallest n 
                int64_t min_burst = findSmallestBurst(burstsLeft, quantum,rq);
                if(min_burst != -1)
                {
                    int64_t next_arrival = processes[jq.front()].arrival - curr_time - 1;
                    min_burst--;
                    int64_t min;
                    int64_t n;

                    if(min_burst <= next_arrival)
                    {
                        min = min_burst;
                        n = std::round(min / quantum);
                    }
                    else 
                    {
                        min = next_arrival;
                        n = std::round(min / rq.size());
                        n = std::round(n / quantum);
                    }

                    //decrease n by on if needed and set new curr_time
                    if((curr_time + (int64_t)(n * rq.size() * quantum)) >= processes[jq.front()].arrival) n--;
                    curr_time = curr_time + (n * rq.size() * quantum);


                    //update bursts for all rq 
                    for(auto & r : rq)
                    {
                        burstsLeft[r] = burstsLeft[r] - (n * quantum);
                    }
                }
            }  
        }
        else if(cpu == -1 && !rq.empty() && jq.empty() && (int64_t)seq.size() == max_seq_len)   //cpu is idle and we have some jobs left and nothing else is arrving
        {
            //pretty much does the above if statement, but in this cause it increments all processes in rq till the smallest is about to finish
            if( checkProcessStartTime(rq, processes) == true)
            {
                int64_t min_burst = findSmallestBurst(burstsLeft, quantum,rq);
                if(min_burst != -1)
                {
                    min_burst--;
                    int64_t n = std::round(min_burst / quantum);
                    curr_time = curr_time + (n * rq.size() * quantum);

                    //update all bursts
                    for(auto & r : rq)
                    {
                        burstsLeft[r] = burstsLeft[r] - (n * quantum);
                    }
                }
            }
        }
        else if(rq.size() == 1 && cpu == -1 && !jq.empty()) //we have on job and cpu is idle
        {
            //put job on cpu
            cpu = rq.front();
            rq.erase(rq.begin());

            //update seq. Check for consecutive duplicates. Check if size is correct
            if((seq.back() != cpu && (int64_t)(seq.size()) < max_seq_len) || seq.empty())
            {
                seq.push_back(cpu);
            }

            //update start time if needed
            if(processes[cpu].start_time == -1) processes[cpu].start_time = curr_time;


            int64_t next_arrival = processes[jq.front()].arrival - curr_time;   //get next arrival

            if(quantum <= next_arrival) 
            {
                //this is how many times we can increase quantum
                int64_t n = next_arrival / quantum;
                if(burstsLeft[cpu] - quantum * n > 0)   //process wont finish, so subtract n*quantum with safest n
                {
                    burstsLeft[cpu] -= (n * quantum);
                    curr_time += (n * quantum);
                }
                else                                    //process will finish within quantum so just finish it
                {
                    curr_time += burstsLeft[cpu];
                    burstsLeft[cpu] = 0;
                }

                if(burstsLeft[cpu] > 0)                 //check if process is done, if not its is read added to rq
                {                    
                    rq.push_back(cpu);
                    cpu = -1;
                }
            }
            
        }

        //cpu is idle and we have jobs 
        //put job on cpu. This is the worst case for really large bursts and small quantums
        if(cpu == -1 && !rq.empty())
        {
            cpu = rq.front();
            rq.erase(rq.begin());
            continue;
        }
        
        /*
            **HINT
            executing full burst slice and checking if anything arrives in between below
        */
        if(burstsLeft[cpu] > 0)
        {
            //we have a process to execute, check if the process is just arriving, if so record start time 
            if(processes[cpu].start_time == -1) processes[cpu].start_time = curr_time;

            if(burstsLeft[cpu] - quantum >= 0)  //**HINT execute slice then check for arriving process
            {
                burstsLeft[cpu] -= quantum;
                curr_time += quantum;
            }
            else                                //**HINT process finishes before quantum, advance time to end of process, then check for arriving process
            {
                curr_time += burstsLeft[cpu];
                burstsLeft[cpu] = 0;
            }

        }
        else 
        {
            continue;
        }
        
        //check if anything arrived
        if(!jq.empty())
        {
            int counter = 0;
            for(auto & y : jq)
            {
                //add new arrivals to rq
                if(curr_time <= processes[y].arrival) break;
                rq.push_back(y);
                counter++;
            }
            if(counter >= 0) jq.erase(jq.begin(), jq.begin() + counter);    //update jq
        }

        if(burstsLeft[cpu] > 0) //check if a process finished
        {
            //not finished to re-add to rq
            rq.push_back(cpu);
            cpu = -1;
        }
    }
}