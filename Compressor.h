/**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009
 * @date    11/11/08
 * @brief   This is the interface for the Compressor.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: Compressor
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <map>
#include <deque>
#include <boost/tuple/tuple.hpp>

#include "Cell.h"
#include "Instruction.h"
#include "param_types.h"
#include "param_funcs.h"

class symbols;
class rules;

/**
 * @ingroup Compressor
 * @brief   Compression utility
 *
 */
class Compressor
{
   public:
      Compressor(deque < Cell * > *cellIn);
      ~Compressor();

      void check(void);
      void compression(void);
      void generateNewCellList(void);
      void printRule(UINT_32 index);
      void printCompleteRuleList(void);

      std::deque < Cell * > *get_reduced_cellList(void);

      void reduceSequential(void);

   protected:


   private:
      UINT_32  wasLoop;
      INT_32   min_terminal;         // minimum and maximum value among terminal symbols
      INT_32   max_terminal;
      INT_32   maxRuleLength;

      string reductionString;                                     //string of cells (this is the pattern matching input)
      rules *S;                                                   //pointer to main rule of the grammar
      rules **R1;
      int Ri;

      std::deque < Cell * > *cellList;
      std::deque < Cell * > *reduced_cellList;
      std::map <UINT_32, std::deque< UINT_32 > > cellMatches;     //map of cell->cell
      std::map <UINT_32, INT_32 > cellAliases;

      void buildRules(void);
      void addCell(symbols *symbolIn);
      UINT_32 checkRules(symbols *symbolIn);
      UINT_32 buildSuperCells(UINT_32 index);

      Cell *generateSeqCell(UINT_32 numInstructions);

      std::deque < UINT_32 > *cellIDs;
      UINT_32 rootIndex;

};

#endif
