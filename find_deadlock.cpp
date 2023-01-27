/*
    Muhammad Razeen Adil 
    CPSC 457
    Assignment 3 - Question 1
*/

#include "find_deadlock.h"
#include "common.h"
#include <unistd.h>
#include <string>
#include <algorithm>    
#include <vector>       
#include <unordered_map>

class Graph {
    std::vector<std::vector<int>> adj_list;
    std::vector<int> out_counts;
    std::vector<std::string> stringNameTracker;     //tracks the names of resources and process in string format

    public:
        void resizeVector(const int &p, const int &r);  //resizes adj_list, out_counts, and stringNameTracker
        void insertEdge(const int &e1, const int &e2);  //inserts an edge
        void trackName(const std::string &str_Process,  const std::string &str_Resource, const int &process, const int &resource);  //stores names
        bool checkProcess(const int &i);    //checks if a string is a process
        std::vector<std::string> sort();    //used to check for a deadlock
};

//check if string doesn't starts with *, if so it's a process return true
//otherwise return false
bool Graph::checkProcess(const int &i)
{
    if(stringNameTracker[i][0] != '*')  //doesnt start with * so it's a process
    {
        return true;
    }
    return false;
}

std::vector<std::string> Graph::sort()
{
    std::vector<int> out = out_counts;
    std::vector<int> zeros;
    std::vector<std::string> returnThis;
    int n;

    int i = 0;
    for(auto & o : out) //all nodes with outdegree == 0
    {
        if(o == 0) zeros.push_back(i);
        ++i;
    }

    //sorting algorithm from hints
    while(!zeros.empty())
    {
        //remove and store last entry
        n = zeros.back();
        zeros.pop_back();

        for(auto & n2 : adj_list[n])    
        {
            if(--out[n2] == 0) zeros.push_back(n2);           
        }
    }

    //checking for deadlock where n is a process and out[n] > 0
    int j = 0;
    for(auto & d : out)
    {
        if(d > 0 && checkProcess(j))    //out[n]> 0 and if string at that index is a process
        {
            returnThis.push_back(stringNameTracker[j]);
        }
        ++j;
    }
    return returnThis;
}

//resizes vector based on w2i conversion
//so initally vector is small then it gets bigger
void Graph::resizeVector(const int &p, const int &r)
{
    long unsigned max = std::max(p,r);

    if(max >= adj_list.size())
    {
        //increase all vector sizes 
        adj_list.resize(max + 1);  
        out_counts.resize(max + 1);
        stringNameTracker.resize(max + 1);
    }
}

//inserts an edge into vector, and increments out degree
void Graph::insertEdge(const int &e1, const int &e2) //P -> R //P <- R
{
    adj_list[e1].push_back(e2);
    out_counts[e2]++;
}

//tracks the names of processes and resources
//so we can convert the intergers back to a string
void Graph::trackName(const std::string &str_Process, const std::string &str_Resource, const int &process, const int &resource)
{
    stringNameTracker[process] = (str_Process);
    stringNameTracker[resource] = (str_Resource);
}

Result find_deadlock(const std::vector<std::string> & edges)
{
    Result result;
    Graph graph;
    Word2Int w2i;
    std::vector<std::string> pushThis;
    int counter = -1;


    for(auto & e : edges)
    {
        auto temp = split(e);
        std::string str_Process = (temp[0]);
        std::string str_Resource = ("*" + temp[2]); //mark this string as a resource (*)

        //convert to int
        int process = w2i.get(str_Process);
        int resource = w2i.get(str_Resource);

        //resize and track the converted name
        graph.resizeVector(process, resource);
        graph.trackName(str_Process, str_Resource, process, resource);

        if(temp[1] == "->") //P -> R
        {
            graph.insertEdge(resource, process);
        }
        else     //P <- R
        {
            graph.insertEdge(process, resource);
        }

        pushThis = graph.sort();

        counter++;

        if(! pushThis.empty())  //deadlock detected, return results
        {
            result.procs = pushThis;
            result.index = counter;  
            return result;
        }        
    }

    result.index = -1;
    return result;
}



