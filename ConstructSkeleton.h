/**
 * @file
 * @author  James Poe   <>, (C) 2008, 2009
 * @date    09/09/08
 * @brief   This is the interface for the ConstructSkeleton object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: ConstructSkeleton
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONSTRUCTSKELETON_H
#define CONSTRUCTSKELETON_H

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <list>
#include <set>
#include <iostream>

using std::set;
using std::list;

#include "param_funcs.h"
#include "Skeleton.h"
#include "Config.h"

/**
 * @ingroup ConstructSkeleton
 * @brief   
 *
 */
class ConstructSkeleton {


public:

          ConstructSkeleton ( Config* cf );

          Skeleton createSkeleton( );

private:

          Config*           cf;                                         // Pointer to configuration

          void              populateThread ( string thread, THREAD_CELL_DEQUEP tdeq );
          unsigned int      calculateMinimumCellCount ( string thread );


          long              getRandomNumber ( long min, long max );
          long              getGreatestCommonDivisor ( long num1, long num2);
          set<long>         getPrimeFactors ( long num );
          long              getGreatestCommonDivisor ( list<long> nums );
          set<long>         getFactors ( long n );

};

// Temporary Method for creating Random Numbers
inline long ConstructSkeleton::getRandomNumber ( long min, long max )
{
  return ( ( rand() % max ) + min );
}

// Euclid Greatest Common Divisor of 2 Numbers Function
inline long ConstructSkeleton::getGreatestCommonDivisor ( long num1, long num2 )
{
  return ( num2 != 0 ? getGreatestCommonDivisor( num2, num1 % num2 ) : num1 );
}

// Comparator for qsort sorting of Cells based upon Instruction Size
inline int instCountComparator ( const void * elem1, const void * elem2 )
{
  return ( ( * ( Cell * * ) elem1 )->getNumInstructions() - ( * ( Cell * * ) elem2 )->getNumInstructions() );
}

// Function to get all of the factors of a number
inline set<long> ConstructSkeleton::getFactors ( long n )
{
  // 1 is a factor of any positive integer
  set<long> factors;
  factors.insert(1);

  long max = n;

  // begin finding factors
  if (n % 2 == 0)
  {
    // n is even
    factors.insert(2);
    for (long factor = 3; factor <= max; ++factor)
    {
      if (n % factor == 0)
      {
        factors.insert(factor);
      }
    }
  }
  else
  {
    // n is odd, so it has no even factors
    for (long factor = 3; factor <= max; factor += 2)
    {
      if (n % factor == 0)
      {
        factors.insert(factor);
      }
    }
  }

  return factors;
}

// Function used to get all prime factors of a number n (not used anymore)
inline set<long> ConstructSkeleton::getPrimeFactors ( long num )
{
  long i;                 /* counter */
  long c;                 /* remaining product to factor */

  set < long > result;

  c = num;

  while ( ( c % 2 ) == 0)
  {
    result.insert(2);
    c = c / 2;
  }

  i = 3;

  while (i <= ( sqrt ( c ) + 1 ) )
  {
    if ( ( c % i ) == 0 ) 
    {
      result.insert ( i );
      c = c / i;
    }
    else
    {
      i = i + 2;
    }

    if (c > 1)
    {
      result.insert ( c );
    }
  }

  return result;
}

// Function to the greatest common divisor of a list of numbers
inline long ConstructSkeleton::getGreatestCommonDivisor ( list<long> nums )
{
  set < long > result;
  set < long > tempResult;
  set < long > temp;
  set <long>::iterator it;
  long max = 1;
  long n;

  if ( nums.empty() )
  {
    return 0;
  }

  result = getFactors ( nums.front() );
  tempResult = result;
  nums.pop_front();

  while ( !nums.empty() )
  {
    temp = getFactors ( nums.front() );
    nums.pop_front();
    for ( it = result.begin(); it != result.end(); it++)
    {
      if ( temp.find ( *it ) == temp.end() )
      {
        tempResult.erase ( *it );
      }
    }
    // This is done as a precaution so we aren't removing a result from a list
    // that an interator is currently traversing
    result = tempResult;
  }

  // Grab the largest value
  for ( it = result.begin(); it != result.end(); it++)
  {
    if ( max < *it )
    {
      max = *it;
    }
  }

  return max;
}

#endif
