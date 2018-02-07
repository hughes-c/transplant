/**
 * @file
 * @author  James Poe   <>, (C) 2008, 2009
 * @date    09/19/08
 * @brief   This is the interface for the Skeleton object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: Skeleton
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SKELETON_H
#define SKELETON_H

#include <deque>

#include "Cell.h"
#include "Config.h"
#include "utilities/nanassert.h"

typedef   deque < Cell * > *    THREAD_CELL_DEQUEP;                                    // Type used with getThread method

/**
 * @ingroup Skeleton
 * @brief   Skeleton container class
 *
 */
class Skeleton {


public:

         Skeleton ( );
         Skeleton ( unsigned int numTheads );
         Skeleton ( unsigned int numTheads, Config* cf );
         Skeleton(const Skeleton &objectIn);

         unsigned int        getNumThreads ( );                                        // Return number of instructions
         deque < Cell * >  * getThread ( unsigned int );                               // Get the deque of cells for a thread
         bool                setThread ( deque < Cell * >  *cell, unsigned int n );    // Set the deque of cells for a thread

         deque < deque < Cell * > > & get_threadList(void);


protected:

         Config*                       configuration;
         unsigned int                  numThreads;                                     // Total number of threads
         deque < deque < Cell * > >    thread;

};


#endif

