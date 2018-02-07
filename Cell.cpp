/**
 * @file
 * @author  James Poe & Clay Hughes   <>, (C) 2008, 2009, 2010
 * @date    09/19/08
 * @brief   This is the implementation for the Cell structure.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: Cell
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include "Cell.h"

Cell::Cell ( )
{
  this->numInstructions = 0;
  this->numUniqueReads = 0;
  this->numUniqueWrites = 0;
  this->numSharedReads = 0;
  this->numSharedWrites = 0;
  this->numMemoryOps = 0;
  this->numIntegerOps = 0;
  this->numFloatingPointOps = 0;
  this->cellType = Sequential;
  this->conflictModel = Random;

   /* basic block size is fixed here */
  this->basicBlockSize = 8;
  this->loopCount = 0;
}

Cell::Cell(const Cell &objectIn)
{
  numInstructions = objectIn.numInstructions;
  numUniqueReads = objectIn.numUniqueReads;
  numUniqueWrites = objectIn.numUniqueWrites;
  numSharedReads = objectIn.numSharedReads;
  numSharedWrites = objectIn.numSharedWrites;
  numMemoryOps = objectIn.numMemoryOps;
  numIntegerOps = objectIn.numIntegerOps;
  numFloatingPointOps = objectIn.numFloatingPointOps;
  cellType = objectIn.cellType;
  conflictModel = objectIn.conflictModel;
}

BOOL Cell::setNumInstructions ( UINT_64 val )
{
  this->numInstructions = val;
  return true;
}

BOOL Cell::setNumUniqueReads ( UINT_64 val )
{
  this->numUniqueReads = val;
  return true;
}

BOOL Cell::setNumUniqueWrites ( UINT_64 val )
{
  this->numUniqueWrites = val;
  return true;
}

BOOL Cell::setNumSharedReads ( UINT_64 val )
{
  this->numSharedReads = val;
  return true;
}

BOOL Cell::setNumSharedWrites ( UINT_64 val )
{
  this->numSharedWrites = val;
  return true;
}

BOOL Cell::setNumMemoryOps ( UINT_64 val )
{
  this->numMemoryOps = val;
  return true;
}

BOOL Cell::setNumIntegerOps ( UINT_64 val )
{
  this->numIntegerOps = val;
  return true;
}

BOOL Cell::setNumFloatingPointOps ( UINT_64 val )
{
  this->numFloatingPointOps = val;
  return true;
}

BOOL Cell::setConflictModel ( ConflictType val )
{
  this->conflictModel = val;
  return true;
}

BOOL Cell::setCellType ( CellType val )
{
  this->cellType = val;
  return true;
}

UINT_64 Cell::getNumInstructions ( ) const
{
  return numInstructions;
}

UINT_64 Cell::getNumUniqueReads ( ) const
{
  return numUniqueReads;
}

UINT_64 Cell::getNumUniqueWrites ( ) const
{
  return numUniqueWrites;
}

UINT_64 Cell::getNumSharedReads ( ) const
{
  return numSharedReads;
}

UINT_64 Cell::getNumSharedWrites ( ) const
{
  return numSharedWrites;
}

UINT_64 Cell::getNumMemoryOps ( ) const
{
  return numMemoryOps;
}

UINT_64 Cell::getNumIntegerOps ( ) const
{
  return numIntegerOps;
}

UINT_64 Cell::getNumFloatingPointOps ( ) const
{
  return numFloatingPointOps;
}

ConflictType Cell::getConflictModel ( ) const
{
  return conflictModel;
}

CellType Cell::getCellType ( ) const
{
  return cellType;
}

BOOL Cell::isTransaction ( )
{
  return ( cellType == 1 );
}

BOOL Cell::isSequential ( )
{
  return ( cellType == 0 );
}

BOOL Cell::isConflictModelHigh ( )
{
  return ( conflictModel == 1 );
}

BOOL Cell::isConflictModelRandom ( )
{
  return ( conflictModel == 0 );
}

BOOL Cell::setBasicBlockSize(UINT_32 basicBlockSize)
{
   this->basicBlockSize = basicBlockSize;
   return 1;
}

UINT_32 Cell::getBasicBlockSize(void) const
{
   return this->basicBlockSize;
}

BOOL Cell::setLoopCount(UINT_32 loopCount)
{
   this->loopCount = loopCount;
   return 1;
}

UINT_32 Cell::getLoopCount(void) const
{
   return this->loopCount;
}

/**
 * @name set_loadConflictList
 * 
 * @param inList 
 * @return 
 * @note This function takes a reference as an argument so the input list should be pointers or copied then updated or there will be corruption
 */
BOOL Cell::set_loadConflictList(std::list< CONFLICT_PAIR > *inList)
{
   loadConflictList = inList;
   return 1;
}

/**
 * @name set_storeConflictList
 * 
 * @param inList 
 * @return 
 * @note This function takes a reference as an argument so the input list should be pointers or copied then updated or there will be corruption
 */
BOOL Cell::set_storeConflictList(std::list< CONFLICT_PAIR > *inList)
{
   storeConflictList = inList;
   return 1;
}

std::list< CONFLICT_PAIR >* Cell::get_loadConflictList(void) const
{
   return this->loadConflictList;
}

std::list< CONFLICT_PAIR >* Cell::get_storeConflictList(void) const
{
   return this->storeConflictList;
}

BOOL Cell::delete_loadConflictList(void)
{
   delete loadConflictList;
}

BOOL Cell::delete_storeConflictList(void)
{
   delete storeConflictList;
}

BOOL Cell::push_loadConflictList(CONFLICT_PAIR inPair)
{
   loadConflictList->push_back(inPair);
   return 1;
}

BOOL Cell::push_storeConflictList(CONFLICT_PAIR inPair)
{
   storeConflictList->push_back(inPair);
   return 1;
}

BOOL Cell::print_loadConflictList(std::ostream &outputStream)
{
   std::list< CONFLICT_PAIR >::iterator listIterator;

   for(listIterator = loadConflictList->begin(); listIterator != loadConflictList->end(); listIterator++)
   {
      outputStream << "First:  " << listIterator->first << "   " << "Second:  " << listIterator->second << "\n";
   }
   outputStream << std::endl;
}

BOOL Cell::print_storeConflictList(std::ostream &outputStream)
{
   std::list< CONFLICT_PAIR >::iterator listIterator;

   for(listIterator = storeConflictList->begin(); listIterator != storeConflictList->end(); listIterator++)
   {
      outputStream << "First:  " << listIterator->first << "   " << "Second:  " << listIterator->second << "\n";
   }
   outputStream << std::endl;
}

/**
 * @name operator==
 * 
 * @note At the moment we only care about detailed matching for transactional cells
 * @param cell 
 * @return 
 */
BOOL Cell::operator==(const Cell& cell) const
{
   const float TOLERANCE = CELL_TOL;             //how much variability is allowed
   UINT_32 high;
   UINT_32 low;

   //cell type (Tx - Sq)
   if(cellType != cell.cellType)
      return 0;

   //conflict Model (high - rnd)
   if(conflictModel != cell.conflictModel)
      return 0;

   //numInstructions
   high = numInstructions + numInstructions * TOLERANCE;
   low = numInstructions - numInstructions * TOLERANCE;
// std::cout << "*numInstructions High:  " << high << " Low:  " << low << " Cell:  " << cell.numInstructions << "\n";
   if(cell.numInstructions > high || cell.numInstructions < low)
      return 0;

   //numUniqueReads
   if(cellType == Transactional)
   {
      high = numUniqueReads + numUniqueReads * TOLERANCE;
      low = numUniqueReads - numUniqueReads * TOLERANCE;
   // std::cout << "*numUniqueReads High:  " << high << " Low:  " << low << " Cell:  " << cell.numUniqueReads << "\n";
      if(cell.numUniqueReads > high || cell.numUniqueReads < low)
         return 0;
   }

   //numUniqueWrites
   if(cellType == Transactional)
   {
      high = numUniqueWrites + numUniqueWrites * TOLERANCE;
      low = numUniqueWrites - numUniqueWrites * TOLERANCE;
   // std::cout << "*numUniqueWrites High:  " << high << " Low:  " << low << " Cell:  " << cell.numUniqueWrites << "\n";
      if(cell.numUniqueWrites > high || cell.numUniqueWrites < low)
         return 0;
   }

   //numSharedReads
   if(cellType == Transactional)
   {
      high = numSharedReads + numSharedReads * TOLERANCE;
      low = numSharedReads - numSharedReads * TOLERANCE;
   // std::cout << "*numSharedReads High:  " << high << " Low:  " << low << " Cell:  " << cell.numSharedReads << "\n";
      if(cell.numSharedReads > high || cell.numSharedReads < low)
         return 0;
   }

   //numSharedWrites
   if(cellType == Transactional)
   {
      high = numSharedWrites + numSharedWrites * TOLERANCE;
      low = numSharedWrites - numSharedWrites * TOLERANCE;
   // std::cout << "*numSharedWrites High:  " << high << " Low:  " << low << " Cell:  " << cell.numSharedWrites << "\n";
      if(cell.numSharedWrites > high || cell.numSharedWrites < low)
         return 0;
   }

   //numMemoryOps
   if(cellType == Transactional)
   {
      high = numMemoryOps + numMemoryOps * TOLERANCE;
      low = numMemoryOps - numMemoryOps * TOLERANCE;
   // std::cout << "*numMemoryOps High:  " << high << " Low:  " << low << " Cell:  " << cell.numMemoryOps << "\n";
      if(cell.numMemoryOps > high || cell.numMemoryOps < low)
         return 0;
   }

   return 1;
}

/**
 * @ingroup Cell
 * @brief Write an internal configuration file to output stream
 * 
 * @param os 
 * @param cell 
 * @return 
 */
std::ostream& operator<<( std::ostream& os, const Cell& cell )
{

  os << "\nnumInstructions:\t" << cell.getNumInstructions ( );
  os << "\nnumUniqueReads:\t\t" << cell.getNumUniqueReads ( );
  os << "\nnumUniqueWrites:\t" << cell.getNumUniqueWrites ( );
  os << "\nnumSharedReads:\t\t" << cell.getNumSharedReads ( );
  os << "\nnumSharedWrites:\t" << cell.getNumSharedWrites ( );
  os << "\nnumMemoryOps:\t\t" << cell.getNumMemoryOps ( );
  os << "\nnumIntegerOps:\t\t" << cell.getNumIntegerOps ( );
  os << "\nnumFloatingPointOps:\t" << cell.getNumFloatingPointOps ( );
  os << "\ncellType:\t\t" << cell.getCellType ( );
  os << "\nconflictModel:\t\t" << cell.getConflictModel ( );
  os << "\nbasicBlockSize:\t\t" << cell.getBasicBlockSize ( );

  std::list< CONFLICT_PAIR > *loadConflictList = cell.get_loadConflictList ( );
  std::list< CONFLICT_PAIR > *storeConflictList = cell.get_storeConflictList ( );

  std::list< CONFLICT_PAIR >::iterator it;

  os << "\nRead Set:";

  for ( it = loadConflictList->begin() ; it != loadConflictList->end() ; it++ )
  {
    os << "\n" << std::hex << it->first << "\t" << std::dec << it->second;
  }

  os << "\nWrite Set:";

  for ( it = storeConflictList->begin() ; it != storeConflictList->end() ; it++ )
  {
    os << "\n" << std::hex << it->first << "\t" << std::dec << it->second;
  }

  return os;
}

