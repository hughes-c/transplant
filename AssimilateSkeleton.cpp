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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <cmath>

#include "AssimilateSkeleton.h"
#include "Cell.h"


AssimilateSkeleton::AssimilateSkeleton(const char* filename)
{
  infile.open(filename); // opens the file

  if(!infile) { // file couldn't be opened
    cerr << "Error: file could not be opened" << endl;
    exit(1);
  }

  numThreads = 0;

}




Skeleton AssimilateSkeleton::loadSkeleton()
{
  std::cout << "Loading skeleton..." << std::endl;

  Skeleton skel;

  string line;

  while( getline( infile, line ) )
  {
    vector<string>  tokenizedLine;
    vector<string>  tokenizedReadSet;
    vector<string>  tokenizedWriteSet;

    Tokenize( line, tokenizedLine, ":" );

    if(tokenizedLine.size() > 1)
    {
      if((tokenizedLine[0] == "<threadCount>"))
      {
        numThreads = atoi(tokenizedLine[1].c_str());
        skel = Skeleton ( numThreads );
      }

      if((tokenizedLine[0] == "<cell>"))
      {
        Cell *tempCell = new Cell;

        switch ( atoi(tokenizedLine[2].c_str( ) ) )
        {
          case 0: tempCell->setCellType( Sequential ); break;
          case 1: tempCell->setCellType( Transactional ); break;
          default: cout << "\nFatal Error: Unknown Cell Type" << endl; break;
        }

        tempCell->setNumInstructions( strtoull(tokenizedLine[3].c_str(),NULL,10) );
        tempCell->setNumUniqueReads( strtoull(tokenizedLine[4].c_str(),NULL,10) );
        tempCell->setNumUniqueWrites( strtoull(tokenizedLine[5].c_str(),NULL,10) );
        tempCell->setNumSharedReads( strtoull(tokenizedLine[6].c_str(),NULL,10) );
        tempCell->setNumSharedWrites( strtoull(tokenizedLine[7].c_str(),NULL,10) );
        tempCell->setNumMemoryOps( strtoull(tokenizedLine[8].c_str(),NULL,10) );
        tempCell->setNumIntegerOps( strtoull(tokenizedLine[9].c_str(),NULL,10) );
        tempCell->setNumFloatingPointOps( strtoull(tokenizedLine[10].c_str(),NULL,10) );

        switch ( atoi(tokenizedLine[11].c_str( ) ) )
        {
          case 0: tempCell->setConflictModel( Random ); break;
          case 1: tempCell->setConflictModel( High ); break;
          case 2: tempCell->setConflictModel( Specified ); break;
          default: cout << "\nFatal Error: Unknown Conflict Model" << endl; break;
        }

        if ( tokenizedLine.size() > 12 )
        {

           std::list< CONFLICT_PAIR > *loadConflictList = new list<CONFLICT_PAIR>;
           std::list< CONFLICT_PAIR > *storeConflictList =  new list<CONFLICT_PAIR>;

          Tokenize ( tokenizedLine[12], tokenizedReadSet, "{}" );
          Tokenize ( tokenizedLine[13], tokenizedWriteSet , "{}" );

          vector<string>::iterator it;
          vector<string> tmp;

          for ( it = tokenizedReadSet.begin() ; it < tokenizedReadSet.end() ; it++ )
          {
            tmp.clear();
            Tokenize ( *it, tmp, "," );

            if ( tmp.size() > 1 )
            {
              loadConflictList->push_back( pair < ADDRESS_INT, UINT_32 > ( strtoull(tmp[0].c_str(),NULL,16) , strtoull(tmp[1].c_str(),NULL,10) )  );
            }

          }

          for ( it = tokenizedWriteSet.begin() ; it < tokenizedWriteSet.end() ; it++ )
          {
            tmp.clear();

            Tokenize ( *it, tmp, "," );

            if ( tmp.size() > 1 )
            {
              storeConflictList->push_back( pair < ADDRESS_INT, UINT_32 > ( strtoull(tmp[0].c_str(),NULL,16) , strtoull(tmp[1].c_str(),NULL,10) )  );
            }

          }

          tempCell->set_loadConflictList ( loadConflictList );
          tempCell->set_storeConflictList ( storeConflictList );

        }


        skel.getThread( atoi(tokenizedLine[1].c_str( ) ) )->push_back ( tempCell );
      }

    }
  }


  return skel;

}

