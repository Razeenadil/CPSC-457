/*
  Muhammad Razeen Adil
  CPSC 457
  Assignment 5
*/

#include "fatsim.h"
#include <iostream>
#include <algorithm>
#include <stack>


class Graph {
    std::vector<std::pair<std::vector<long>, long>> adj_list;
    std::vector<long> visited;    //the node we already visited 
    std::vector<long> result;     //result stored here
    long maxChainLength = 0;      //longest chain
    long chainLength = 0;         //current chain length 
    std::stack<long> st;

    public:
    std::vector<long> endOfChain;                     //list of nodes that point to -1 
    void populateAdj(const std::vector<long> & fat);  //populates our adj_list and visted vectors
    std::vector<long> DFS();               
    void checkParentSize(const long & currVal);
    void setChainLength(const long & currVal);           

};

//sets the chain length at a value
void Graph::setChainLength(const long & currVal)
{
  if (adj_list[currVal].second != -1 && adj_list[currVal].second != 0)  
  {
    chainLength = adj_list[currVal].second;
  }
}

//checks to see if parent has multiple children, and sets the chain length at that point
void Graph::checkParentSize(const long & currVal)
{
  if (adj_list[currVal].first.size() > 1)
  {
    adj_list[currVal].second = chainLength; 
    for(auto & h : adj_list[currVal].first)
    {
      adj_list[h].second = chainLength + 1;
    }
  }
}

//this function populates our adj_list
void Graph::populateAdj(const std::vector<long> & fat)
{
  std::pair<std::vector<long>, long> tPair;
  std::vector<long> temp;
  long counter = 0;
  adj_list.resize(fat.size());
  visited.resize(fat.size(), -1);

  for(const auto & f : fat)
  {
    if(f != -1)   //node does not point to -1
    {
      //gets the previous elements from adj_list and adds the new element to it
      temp = adj_list[f].first;     
      temp.push_back(counter);
      tPair = std::make_pair(temp, -1);   //second element in pair is initalized to -1
      adj_list[f] = tPair;
      counter++;
      continue;
    }

    //a node that is pointing to -1 gets added to endOfChain
    endOfChain.push_back(counter);
    counter++;      
  }
}



//used to loop through endOfChain
std::vector<long> Graph::DFS()
{
  for(const auto & eoc : endOfChain)
  {
    if(visited[eoc] != -1) continue;  //we already visited that node. (this will never happen tho....i think)
    st.push(eoc);
    chainLength++;

    while(1)
    {  
      long currVal = st.top();
      st.pop();

      setChainLength(currVal);
      checkParentSize(currVal);

      if (adj_list[currVal].first.empty())    //we reached the first element in the chain
      {
        if(chainLength > maxChainLength)      //we have a new max chain length
        {
          //update result and max
          maxChainLength = chainLength;
          result.clear();
          result.push_back(currVal);
        }
        else if(chainLength == maxChainLength)  //update result. chainLength is same as max
        {
          result.push_back(currVal);
        }
        
        if(st.empty())
        {
          chainLength = 0;
          break;
        } 
        continue;
      } 

      //get the next element and add it to the stack
      for(const auto & a : adj_list[currVal].first)
      {
        if(visited[a] != -1) continue;
      
        setChainLength(currVal);
        st.push(a);
        chainLength++;
      }
    }
  }

  //sort result and return
  std::sort(result.begin(), result.end());
  return result;
}


std::vector<long> fat_check(const std::vector<long> & fat)
{
  Graph graph;
  graph.populateAdj(fat);
  if(graph.endOfChain.empty()) return {};   //we dont have a chain that points -1 
  return graph.DFS();
}