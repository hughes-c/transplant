/**
 * @file
 * @author  James Poe   <>, (C) 2008, 2009
 * @date    09/09/08
 * @brief   This is the implementation for the Skeleton object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: Skeleton
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include "Skeleton.h"

/**
 * @ingroup Skeleton
 */
Skeleton::Skeleton ( )
{
  Skeleton ( 0 );
}

/**
 * @ingroup Skeleton
 * 
 * @param numThreads 
 */
Skeleton::Skeleton ( unsigned int numThreads )
{
  this->numThreads = numThreads;

  for ( unsigned int i = 0; i < numThreads; i++ )
  {
    deque < Cell * > newThread;
    thread.push_back ( newThread );
  }
}

/**
 * @ingroup Skeleton
 * 
 * @param numThreads 
 * @param cf Configuration pointer
 */
Skeleton::Skeleton ( unsigned int numThreads, Config* cf )
{
  this->numThreads = numThreads;
  this->configuration = cf;

  for ( unsigned int i = 0; i < numThreads; i++ )
  {
    deque < Cell * > newThread;
    thread.push_back ( newThread );
  }
}

/**
 * @ingroup Skeleton
 * 
 * @param objectIn Skeleton to copy values
 */
Skeleton::Skeleton(const Skeleton &objectIn)
{
   numThreads = objectIn.numThreads;
   thread = objectIn.thread;
}

/**
 * @ingroup Skeleton
 * 
 * @return 
 */
unsigned int Skeleton::getNumThreads ( )
{
  return numThreads;
}

/**
 * @ingroup Skeleton
 * 
 * @param n 
 * @return Deque of Cells
 */
deque < Cell * >  * Skeleton::getThread ( unsigned int n )
{
  I( n < numThreads );
  return &thread [ n ];
}

/**
 * @ingroup Skeleton
 * 
 * @param cell Deque of Cells
 * @param n 
 * @return bool
 */
bool Skeleton::setThread ( deque < Cell * >  *cell, unsigned int n )
{
  I( n < numThreads );
  thread[ n ] = *cell;
  return 1;
}

/**
 * @ingroup Skeleton
 * 
 * @return Deque of Cells
 */
deque < deque < Cell * > > & Skeleton::get_threadList(void)
{
   return this->thread;
}
