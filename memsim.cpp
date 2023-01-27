/*
  Muhammad Razeen Adil
  Assignment 6
  I didn't do any optimization :(
*/

#include "memsim.h"
#include <cassert>
#include <iostream>
#include <list>
#include <cmath>

struct Partition {
  int tag;
  int64_t size, addr;
};

typedef std::list<Partition>::iterator PartitionRef;


struct Simulator {
  std::list<Partition> all_blocks;
  int64_t pageSize = 0;

  Simulator(int64_t page_size)
  {
    pageSize = page_size;
  }
  void allocate(int tag, int size)
  {
    Partition part;
    PartitionRef pRef;
    
    if(!all_blocks.empty())
    {
      int64_t maxSize = 0;
      //get the free block with the largest size
      for(auto x = all_blocks.begin(); x != all_blocks.end(); x++)
      {
        if(x -> tag == -1 && x -> size > maxSize)
        { 
          maxSize = x -> size;
          pRef = x;
        }
      }

      //insert in all_blocks one before the the max 
      if(maxSize >= 0)
      {                 
        if(size == maxSize)
        {
          //add new block to end of list 
          all_blocks.back().tag = tag;
          all_blocks.back().size = size;

          //upadte the address, 0 if first item in list
          if(pRef == all_blocks.begin()) all_blocks.back().addr = 0;
          
          if(pRef != all_blocks.begin()) all_blocks.back().addr = std::prev(pRef) -> addr + std::prev(pRef) -> size;
           
        }
        if(size < maxSize)
        {
          part.tag = tag;
          part.size = size;

          //upadte the address, 0 if first item in list
          if(pRef == all_blocks.begin()) part.addr = 0;
          
          if(pRef != all_blocks.begin()) part.addr = std::prev(pRef) -> addr + std::prev(pRef) -> size;

          all_blocks.insert(pRef, part);    //insert new values at the iterator

          if(std::next(pRef) == all_blocks.end())
          {
            //update the free block
            all_blocks.back().tag = -1;
            all_blocks.back().size = maxSize - size;
            all_blocks.back().addr = part.addr + size;
          }
          else 
          {
            //not at the end of list
            pRef -> tag = -1;
            pRef -> size = maxSize - size;
            pRef -> addr = part.addr + size;
          }
        }
        else if (size > maxSize)
        {
          pRef = all_blocks.end();  //get new iterator values

          if(size % pageSize == 0)  //page size is perfect
          {
            if(all_blocks.back().tag == -1)
            {
              //update at the end position
              all_blocks.back().tag = tag;
              all_blocks.back().size = size;
            }
            else 
            {
              //create new partition and insert it 
              part.tag = tag;
              part.size = size;
              part.addr = all_blocks.back().addr + all_blocks.back().size;
              all_blocks.push_back(part);
            }
          }
          else 
          {
            int64_t t = pageSize - ((size - all_blocks.back().size) % pageSize);

            if(all_blocks.back().tag == -1)
            {
              //update end block, address will stay the same
              all_blocks.back().tag = tag;
              all_blocks.back().size = size;
            }
            else 
            {
              //create new partition and insert it at the end
              part.tag = tag;
              part.size = size;
              part.addr = std::prev(pRef) -> size + std::prev(pRef) -> addr;
              all_blocks.push_back(part);
            }

            //for updateing the free block at the end
            if(size < pageSize && maxSize != 0)
            {
              //create new partition and insert it at the end
              part.tag = -1;
              part.size = pageSize - ((-1 * (maxSize - size)) % pageSize);
              part.addr = all_blocks.back().addr + size;
              all_blocks.push_back(part);
            }
            else if(size > pageSize && maxSize != 0)
            {
              //create new partition and insert it at the end
              part.tag = -1;
              part.size = t;
              part.addr = all_blocks.back().addr + size;
              all_blocks.push_back(part);
            }
            else if(maxSize == 0) 
            {
              //create new partition and insert it at the end
              part.tag = -1;
              part.size = pageSize - ((-1 * (maxSize - size)) % pageSize);
              part.addr = all_blocks.back().addr + size;
              all_blocks.push_back(part);
            }           
          }          
        }
      } 
    }
    else if(all_blocks.empty())
    {
      if(size % pageSize == 0)
      {
        //set up our partication values
        part.tag = tag;
        part.size = size;
        part.addr = 0;

        //insert into all_blocks
        all_blocks.push_back(part);
      }
      else
      {
        //set up our partication values
        part.tag = tag;
        part.size = size;
        part.addr = 0;       
        
        //insert into all_blocks
        all_blocks.push_back(part);

        //insert the free block
        int64_t t = std::ceil((double)(size) / (double)(pageSize));
        part.tag = -1; 
        part.addr = size;
        part.size = (t * pageSize) - size;
        all_blocks.push_back(part);       
      }
    }
  }

  void deallocate(int tag)
  {
    PartitionRef pRef;
    for(auto x = all_blocks.begin(); x != all_blocks.end(); x++)  //loop through partitions
    {
      pRef = x;

      if(x -> tag == tag || x -> tag == -1) //matching tags 
      {
        x -> tag = -1;  //upate tag

        //if we are not at the first element, check if previous element has -1 tag so we can combine
        if(x != all_blocks.begin() && std::prev(pRef) -> tag == -1 && pRef -> tag == -1)
        {
          //update tag and the size
          x -> tag = -1;
          x -> size = x -> size + std::prev(pRef) -> size;

          //update the address
          if(pRef == all_blocks.begin() || std::prev(pRef) == all_blocks.begin())
          { 
            x -> addr = 0;  //first element has addr of 0
          } 
          else 
          {
            x -> addr = std::prev(pRef) -> addr;  //calculate the addr
          }
          all_blocks.erase(std::prev(pRef));    //earse free blocks after combining them
        }
      } 
    }
  }
 
  MemSimResult getStats()
  {
    MemSimResult result;
    result.max_free_partition_size = 0;
    result.max_free_partition_address = 0;
    result.n_pages_requested = 0;
    int64_t total = 0;
    bool hasAlloc = false;

    //get the results
    for(auto & h : all_blocks)
    {
      total += h.size;
      if(h.tag == -1)
      {
        if(h.size > result.max_free_partition_size)
        {
          result.max_free_partition_size = h.size;
          result.max_free_partition_address = h.addr;
        } 
      }
      else 
      {
        hasAlloc = true;
      }
    }

    if(hasAlloc)
    {
      result.n_pages_requested = std::ceil((double)total / (double)pageSize);
    }

    return result;
  }
};


MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  Simulator sim(page_size);
  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }
  }
  return sim.getStats();
}
