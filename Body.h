/**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009, 2010
 * @date    09/19/08
 * @brief   This is the interface for the Body module.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: Body \n
 * This file is responsible for writing the final output flies. It takes a Skin object
 * as a constructor and uses the high-level definitions there to generate the final
 * output files.
 *
 * The defines at the top are used to determine the size of the memory pools.
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////



#ifndef BODY_H
#define BODY_H

#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <boost/tuple/tuple.hpp>

#include "Skin.h"
#include "OperandList.h"
#include "param_types.h"
#include "param_funcs.h"

#define INT_SIZE     4
#define CACHE_LINE  32

#define MAX_UNIQUE 2048 * 4
#define MEM_REGION MAX_UNIQUE * CACHE_LINE

// #define MAX_UNIQUE 256
// #define MEM_REGION (MAX_UNIQUE * CACHE_LINE)
#define MAX_MEM 4 * MEM_REGION


/**
 * @ingroup Body
 * @brief   Writer module for output
 *
 */
class Body : public Skin
{
   public:
      /* Data */

      /* Methods */
      Body();
      Body(Skin &skinIn);

      void writeProgram(void);

      void writeInstruction(std::ofstream &outputFile, Instruction *instructionIn, OperandList &operandList);
      void translateInstruction(THREAD_ID threadID, Instruction *instructionIn, OperandList &operandList);

      static UINT_32 randMemory(UINT_32 min, UINT_32 max);

   protected:


   private:
      /* Data */
      UINT_32 basicBlockLabel;

      UINT_32 globalBase;
      UINT_32 maxGlobalOffset;
      UINT_32 privateBase;
      UINT_32 maxPrivateOffset;

      UINT_32 globalLoadBase;
      UINT_32 privateLoadBase;
      UINT_32 globalStoreBase;
      UINT_32 privateStoreBase;

      BOOL privLoad;
      BOOL privStore;
      BOOL sharedLoad;
      BOOL sharedStore;

      BOOL noOverlap;

      INT_32 currentLockedOffset;

      std::map< UINT_32, ADDRESS_INT > *addressMatch;

      std::vector< UINT_32 > *privateStoreOffsetList;
      std::vector< UINT_32 > *globalStoreOffsetList;
      std::vector< UINT_32 > *privateLoadOffsetList;
      std::vector< UINT_32 > *globalLoadOffsetList;

      /* Methods */
      void writeLabel(std::ofstream &outputFile, THREAD_ID threadID);
      void insertBarrier(std::ofstream &outputFile, UINT_32 numThreads);
      void insertWait(std::ofstream &outputFile, UINT_32 numThreads);

      void startTransSection(std::ofstream &outputFile, TX_ID transID);
      void endTransSection(std::ofstream &outputFile, TX_ID transID);

      void initCellLoopSection(std::ofstream &outputFile);
      void beginCellLoopSection(std::ofstream &outputFile, THREAD_ID threadID);
      void endCellLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 iterations);

      void beginMultiCellLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 loopID);
      void endMultiCellLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 loopID, UINT_32 iterations);

      void beginBlockLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 loopID);
      void endBlockLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 loopID, UINT_32 iterations);

      void beginProgamIterations(std::ofstream &outputFile, THREAD_ID threadID);
      void endProgamIterations(std::ofstream &outputFile);

      void headerGen(std::ofstream &outputFile);
      void trailerGen(std::ofstream &outputFile);

      void funcHeaderGen(THREAD_ID threadID, std::ofstream &outputFile);
      void funcTrailerGen(THREAD_ID threadID, std::ofstream &outputFile);
};

#endif
