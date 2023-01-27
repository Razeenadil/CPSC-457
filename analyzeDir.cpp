/*
  Muhammad Razeen Adil 
  Assignment 2 
  CPSC 457
*/

#include "analyzeDir.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <set>
#include <string>
#include <vector>
#include <cstdio>
#include <algorithm>

using namespace std; 

/// check if path refers to a directory
static bool is_dir(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISDIR(buff.st_mode);
}

/// check if path refers to a file
static bool is_file(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), &buff)) return false;
  return S_ISREG(buff.st_mode);
}

/// check if string ends with another string
static bool ends_with(const std::string & str, const std::string & suffix)
{
  if (str.size() < suffix.size()) return false;
  else
    return 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

/// Got this from https://gitlab.com/cpsc457/public/findLargestDir/-/blob/main/main.cpp
/// check if string starts with another string
static bool starts_with(const std::string & str, const std::string & suffix)
{
  if (str.size() < suffix.size()) return false;
  else
    return 0 == str.compare(0, suffix.size(), suffix);
}

/*
  Comapre function to sort the image struct
*/
bool compare(ImageInfo a, ImageInfo b)
{
  return (a.width * a.height) > (b.width * b.height);
}

/*
  Checks to see if 'fileName' is a text file. 
  Checks the last three characters of 'fileName'.
  Returns true if text file otherwise false.
*/
bool isTextFile(string fileName)
{
  if(ends_with(fileName, ".txt")) return true;

  return false;
}

/*
  If 'path' begins with ./ then the first two characters 
  are removed.
  Returns the updated 'path'
*/
std::string cleanPath(std::string path)
{

  if(starts_with(path, "./"))
  {
    path = path.substr(2);
    return path;
  }
  return path;
}

/*struct to store vacant directories and the total files encountered while searching*/
struct ReturnVacant
{
  long nFiles = 0;
  std::vector<std::string> dirs;
};

/*Struct for storing needed values for return*/
struct ReturnRes 
{
  std::string path = "";
  long largestFile = -1;
  long totalFiles = 0;
  long totalDirs = 1;   //starts at 1, because we take into account the first directory entered by the user
  long totalFileSize = 0;
  std::unordered_map<std::string,int> wordHist;
  std::vector<ImageInfo> images;
}; ReturnRes rr;

/*
  Reads a file 'filer' character by charcter and stores
  them in 'result'.
  A string 'result' is then returned.
  Got this from https://gitlab.com/cpsc457/public/word-histogram/-/blob/master/main.cpp
*/
std::string getNextWord(FILE *filer)
{
  std::string result;
  while(1)
  {
    int c = fgetc(filer);
    if(c == EOF) break;

    c = tolower(c);

    if(! isalpha(c))  //string only contains letters
    {
      if(result.size() == 0) continue;
      break;
    }
    else 
    {
      if(result.size() >= 1023) break;
      result.push_back(c);
    }
  }

  return result;
}

/*
  Opens a stream for reading on given 'path'.
  Stores the returned 'word' in a histogram and updated counter.
  Got this from https://gitlab.com/cpsc457/public/word-histogram/-/blob/master/main.cpp
*/
void readTextFile(string path)
{
  FILE *readStream = fopen(path.c_str(),"r");
  
  while(1)
  {
    auto word = getNextWord(readStream);
    if(word.size() == 0) break;
    if(word.size() < 5) continue;   //word size has to greater than of equal to 5
    rr.wordHist[word]++;
  }

  fclose(readStream);   //close read stream 
}

/*
  Populates the image vector with the 'imageSize' and 'path'
  to that image.
*/
void PopulateImageVector(string path, string imageSize)
{
  std::vector<long> dim;
  ImageInfo II;

  char str [imageSize.length() + 1];
  strcpy(str, imageSize.c_str());

  char *token = strtok(str, " ");   //split string by spaces to get first and second numbers
  while(token != NULL)    
  {
    dim.push_back(stol(token));     //convert and store number in vector
    token = strtok(NULL, " ");
  }

  //assign values to the image struct from analyzeDir.h
  II.path = cleanPath(path);    
  II.width = (long)dim[0];
  II.height = (long)dim[1];

  rr.images.push_back(II);  //push struct to our return vector
}

/*
  Takes a file 'path' and determines if it's a image or not.
  If it is an image then the ImageVector is then populated 
  Got this from https://gitlab.com/cpsc457f22/analyzedir/-/blob/main/analyzeDir.cpp
*/
void checkImage(string path)
{
  string cmd = "identify -format '%w %h' " + cleanPath(path) + " 2> /dev/null";
  std::string imageSize;
  char result[1024];

  auto f = popen(cmd.c_str(), "r");     //call identify command

  if(fgets(result, sizeof(result), f) != NULL)  //read from stdout
  {
    imageSize = result; 
  }

  int status = pclose(f);       //status for checking if image

  if(status != 0 || imageSize[0] == '0' || imageSize.empty())   //not an image
  {
    imageSize = "";
    return;
  }

  PopulateImageVector(path, imageSize);
}

/*
  Takes in a file 'path' and 'name' of the file.
  Runs stat() on the path and gets needed values.
*/
void getFileStats(string path, string name)
{ 
  //setting up structure for stat
  struct stat file_stats;

  if(stat(path.c_str(), & file_stats) != 0) return;
  
  int fileSize = file_stats.st_size;

  //increment totalFiles and totalFileSize
  rr.totalFiles++;
  rr.totalFileSize += fileSize;
  
  if(fileSize > rr.largestFile) //new largest file
  {
    rr.largestFile = fileSize;
    rr.path = path;
  }

  if(isTextFile(name))  //check to see if a file is a text file
  {
    readTextFile(path); //read if text file
  }

  checkImage(path);     //get image dimensions
}


/*
  Stores the vacant dirs in a vector.
*/
ReturnVacant getReturnStruct(string currentDir, long nFiles, std::vector<std::string> empties)
{
  ReturnVacant tt;
  
  if(nFiles == 0) { //no files
    auto dir = opendir(currentDir.c_str());
    if(readdir(dir) == NULL)  //can't read the dir thus all dirs are empty return "."
    {
      tt.nFiles = 0;
      tt.dirs.clear();
      tt.dirs.push_back(".");
    } 
    else
    {
      tt.nFiles = 0;
      tt.dirs.clear();
      tt.dirs.push_back(currentDir);
    }
    closedir(dir);
    return tt;
  }
  else  //there are file
  {
    tt.nFiles = nFiles;
    tt.dirs = empties;
    return tt;
  }
}

/*
  Recursivley read the directories. Takes in a 'currentDir' to check
*/
ReturnVacant recursiveFileSearch(string currentDir)
{
  auto dir = opendir(currentDir.c_str());

  long nFiles = 0;
  std::vector<std::string> empties;

  for (auto de = readdir(dir); de != nullptr; de = readdir(dir)) 
  {
    std::string name = de->d_name;
    if (name == "." or name == "..") continue;    //skip . and ..

    string path = currentDir + "/" + name;
    
    if(is_file(path))   //file encountered
    { 
      nFiles++;     
      getFileStats(path, name);
    } 
    else if (is_dir(path))  //dir encountered
    {
      rr.totalDirs++;

      ReturnVacant res;
      res = recursiveFileSearch(path); //recursive call

      nFiles += res.nFiles;

      for(auto f : res.dirs)    //push vacant dirs
        empties.push_back(f);     
    }
  }

  closedir(dir);
  ReturnVacant tt = getReturnStruct(currentDir, nFiles, empties);

  return tt;
}

/*
  Sorts the wordHist. Takes in the max vector size 'n'
*/
std::vector<std::pair<std::string, int>> getNWord(int n)
{
  std::vector<std::pair<std::string, int>> common;
  std::set<std::pair<int, std::string>> mmq;

  for(auto &h : rr.wordHist)  //sort wordHist
  {
    mmq.emplace(-h.second, h.first);

    if(mmq.size() > size_t(n))  //make sure it is the right size
    {
      mmq.erase(std::prev(mmq.end()));
    }
  }

  for(auto & a : mmq) //sort in vector and return
  {
    common.push_back(make_pair(a.second, -a.first));
  }

  return common;
}


Results analyzeDir(int n)
{
  Results res;
  ReturnVacant r;

  r = recursiveFileSearch(".");

  for(auto x : r.dirs)  //stores the vacant dirs in return vector
  {
    res.vacant_dirs.push_back(cleanPath(x));
  }
 
  //storing values in return struct
  res.largest_file_path = cleanPath(rr.path); 
  res.largest_file_size = rr.largestFile;
  res.n_files = rr.totalFiles;
  res.n_dirs = rr.totalDirs;
  res.all_files_size = rr.totalFileSize;

  if(! rr.wordHist.empty()) //stores the wordHist in return vector 
  {
    res.most_common_words = getNWord(n);
  }


  sort(rr.images.begin(), rr.images.end(), compare);  //sort the image vector

  if(rr.images.size() > size_t(n) && !rr.images.empty())  //set size of image vector to n
  {
    rr.images.resize(n);
  }

  res.largest_images = rr.images;

  return res;
}
