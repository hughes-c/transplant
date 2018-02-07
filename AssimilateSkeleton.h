//
// C++ Implementation: AssimilateSkeleton
//
// Description: 
//
//
/// @Author: James Poe <>, (C) 2008
/// @Date:           09/09/2008
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef ASSIMILATESKELETON_H
#define ASSIMILATESKELETON_H

#include <string>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <deque>


#include "Cell.h"
#include "Skeleton.h"

using namespace std;


class AssimilateSkeleton{

  public:

    AssimilateSkeleton(const char* filename);
    Skeleton loadSkeleton();
    int getNumThreads()
    {
      return numThreads;
    }

  private:

    ifstream infile;
    int numThreads;

    void Tokenize(const string& str, vector<string>& tokens, const string& delimiters = " ")
    {
      // Skip delimiters at beginning.
      string::size_type lastPos = str.find_first_not_of(delimiters, 0);
      // Find first "non-delimiter".
      string::size_type pos     = str.find_first_of(delimiters, lastPos);

      while (string::npos != pos || string::npos != lastPos)
      {
          // Found a token, add it to the vector.
          tokens.push_back(str.substr(lastPos, pos - lastPos));
          // Skip delimiters.  Note the "not_of"
          lastPos = str.find_first_not_of(delimiters, pos);
          // Find next "non-delimiter"
          pos = str.find_first_of(delimiters, lastPos);
      }
    }

};



#endif


