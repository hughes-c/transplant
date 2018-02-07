/**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009
 * @date    09/19/08
 * @brief   This is the interface for the Skin object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: Skin
 * Used to add fidelity to the bare skeleton. It is used to generate memory strides,
 * layout memory locations, develop internal loops, and add synchronization.
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SKIN_H
#define SKIN_H

#include <list>
#include <vector>
#include <deque>
#include <algorithm>
#include <boost/random.hpp>

#include "param_types.h"
#include "param_funcs.h"
#include "Config.h"
#include "Skeleton.h"
#include "Instruction.h"

///NOTE Internal loops require 3 integer ALU ops and 1 branch per cell
#define EXTRA_LOOP_OPS 4

/**
 * @ingroup Skin
 * @brief   Skin container class
 *
 */
class Skin : public Skeleton
{
   public:
      /* Constructor */
      Skin();
      Skin(Skeleton &skeletonIn);
      ~Skin();

      /* Variables */

      /* Functions */
      UINT_32 updateConfig(Config *config);

      void spinalColumn(void);
      void insertVertebrae(void);
      void synchronize(void);

   protected:
      Config *config;
      std::vector< std::deque< Instruction * > * > perThread_instructionList;

   private:
      UINT_32 matchID;

      void makeStrideLoops(std::deque < Instruction * > *instructionList, std::deque< UINT_32 > *conflictDistances);
      void getDistanceList(std::deque < Instruction * > *instructionList, std::list< CONFLICT_PAIR >* readConflictList, std::list< CONFLICT_PAIR >* writeConflictList, std::deque<UINT_32> *conflictDistances);

      void localizeMemory(std::deque < Instruction * > *instructionList, UINT_32 sharedReads, UINT_32 sharedWrites);
      void privatizeMemory(std::deque < Instruction * > *instructionList, UINT_32 sharedReads, UINT_32 sharedWrites);
      void specifyMemory(std::deque < Instruction * > *instructionList, std::list< CONFLICT_PAIR >* readConflictList, std::list< CONFLICT_PAIR >* writeConflictList);
      void prioritizeMemory(std::deque < Instruction * > *instructionList, std::list< CONFLICT_PAIR >* readConflictList, std::list< CONFLICT_PAIR >* writeConflictList);
      void conflictizeMemory(std::deque < Instruction * > *instructionList, UINT_32 uniqueLoads, UINT_32 uniqueStores, BOOL useLoops, UINT_32 iterations, UINT_32 remainder, UINT_32 loopSize);
      void randomizeInstructionStream(std::deque < Instruction * > *instructionList, ConflictType conflictType, CellType cellType, UINT_32 sharedReads, UINT_32 sharedWrites, BOOL useLoops);

};

#endif
