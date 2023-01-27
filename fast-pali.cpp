/*
Muhammad Razeen Adil
A1 - fast-pail.cpp
*/

#include <unistd.h>
#include <stdio.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;



char buffer[1024*1024];     // global variable used to store input read from file
int bufferSize = 0;         //number of character stored 
int bufferPosition = 0;     //position for char buffer, to see where to get character from

//Call read and stores input into buffer.
//Returns -1 when file is read, otherwise returns next element in 
//buffer.
int read_line()
{
  if(bufferPosition >= bufferSize)  //whole buffer has been read 
  {

    bufferSize = read(STDIN_FILENO, buffer, sizeof(buffer));
  
    if(bufferSize <= 0) return -1;  //end of file 

    bufferPosition = 0;   //reset postion
  }
   
  return buffer[bufferPosition++];
}

/*
  This function inserts the charcters from buffer into the string 
  result, does this charcter by character. If EOF is encountered whatever 
  was stored in result is returned. If no EOF is encounter 
  result is returned, when size of result >= 1023 and a space is encounterd.
  @return result - string of retrived from file
*/
std::string stdin_readline()
{
  std::string result;

  while(1)
  {
    int c = read_line();

    if(c == -1) break;  //EOF encountered

    if(result.size() >= 1023) //result string is getting too large
    {
      if(c == 10) break;  //break when "space" is encountered
      result.push_back(c);
      continue;
    }
    result.push_back(c);
  }
  
  return result;
}

/*I got this function from slow-pali.cpp*/
// split string p_line into a vector of strings (words)
// the delimiters are 1 or more whitespaces
std::vector<std::string> split( const std::string & p_line)
{
  auto line = p_line + " ";
  bool in_str = false;
  std::string curr_word = "";
  std::vector<std::string> res;
  for( auto c : line) {
    if( isspace(c)) {
      if( in_str)
        res.push_back(curr_word);
      in_str = false;
      curr_word = "";
    }
    else {
      curr_word.push_back(c);
      in_str = true;
    }
  }
  return res;
}

/*I got this function from slow-pali.cpp*/
// returns true if a word is palindrome
// palindrome is a string that reads the same forward and backward
//    after converting all characters to lower case
bool is_palindrome( const std::string & s)
{
  for( size_t i = 0 ; i < s.size() / 2 ; i ++)
    if( tolower(s[i]) != tolower(s[s.size()-i-1]))
      return false;
  return true;
}

/*I got this function from slow-pali.cpp*/
// Returns the longest palindrome on standard input.
// In case of ties for length, returns the first palindrome found.
//
// Algorithm:
// Input is broken into lines, each line into words, and each word
// is checked to see if it is palindrome, and if it is, whether it
// is longer than the largest palindrome encountered so far.
//
// A word is a sequence of characters separated by white space
// white space is whatever isspace() says it is
//    i.e. ' ', '\n', '\r', '\t', '\n', '\f'
std::string get_longest_palindrome()
{
  std::string max_pali;
  while(1) {
    std::string line = stdin_readline();
    if( line.size() == 0) break;
    auto words = split( line);
    for( auto word : words) {
      if( word.size() <= max_pali.size())
        continue;
      if( is_palindrome(word))
        max_pali = word;
    }
  }
  return max_pali;
}


/*I got this function from slow-pali.cpp*/
int main()
{
  std::string max_pali = get_longest_palindrome();
  printf("Longest palindrome: %s\n", max_pali.c_str());
  return 0;
}
