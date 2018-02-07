/**
 * @file
 * @author  James Poe & Clay Hughes   <>, (C) 2008, 2009, 2010
 * @date    09/19/08
 * @brief   This is the interface for the Cell structure.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: Cell
 *
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CELL_H
#define CELL_H

#include "Config.h"
#include "param_types.h"

///NOTE Cell-matching utiziles a tolerance defined as a percentage (i.e. 0.25)
#define CELL_TOL 0.0

/**
 * @ingroup Cell
 * @brief   
 *
 */
class Cell {


public:
         Cell ( );
         Cell(const Cell &objectIn);

         BOOL              setNumInstructions ( UINT_64 );       // Set number of instructions
         BOOL              setNumUniqueReads ( UINT_64 );        // Set number of unique reads
         BOOL              setNumUniqueWrites ( UINT_64 );       // Set number of unique writes
         BOOL              setNumSharedReads ( UINT_64 );        // Set number of shared reads
         BOOL              setNumSharedWrites ( UINT_64 );       // Set number of shared writes
         BOOL              setNumMemoryOps ( UINT_64 );          // Set number of memory ops
         BOOL              setNumIntegerOps ( UINT_64 );         // Set number of integer opis
         BOOL              setNumFloatingPointOps ( UINT_64 );   // Set number of floating point ops
         BOOL              setConflictModel ( ConflictType );          // Set conflict model
         BOOL              setCellType ( CellType );                   // Set cell type

         UINT_64           getNumInstructions ( ) const;               // Return number of instructions
         UINT_64           getNumUniqueReads ( ) const;                // Return number of unique reads
         UINT_64           getNumUniqueWrites ( ) const;               // Return number of unique writes
         UINT_64           getNumSharedReads ( ) const;                // Return number of shared reads
         UINT_64           getNumSharedWrites ( ) const;               // Return number of shared writes
         UINT_64           getNumMemoryOps ( ) const;                  // Return number of memory ops
         UINT_64           getNumIntegerOps ( ) const;                 // Return number of integer opis
         UINT_64           getNumFloatingPointOps ( ) const;           // Return number of floating point ops
         ConflictType      getConflictModel ( ) const;                 // Return conflict model
         CellType          getCellType ( ) const;                      // Return cell type

         BOOL              isTransaction ( );                          // Return true if cell is transactional
         BOOL              isSequential ( );                           // Return true if cell is sequential

         BOOL              isConflictModelHigh ( );                    // Return true if cell conflict model is high
         BOOL              isConflictModelRandom ( );                  // Return true if cell conflict model is random

         BOOL              setBasicBlockSize(UINT_32 basicBlockSize);
         UINT_32           getBasicBlockSize(void) const;
         BOOL              setLoopCount(UINT_32 loopCount);
         UINT_32           getLoopCount(void) const;

         BOOL operator==( const Cell& cell ) const;

         BOOL                          set_loadConflictList(std::list< CONFLICT_PAIR > *inList);
         BOOL                          set_storeConflictList(std::list< CONFLICT_PAIR > *inList);
         std::list< CONFLICT_PAIR >*   get_loadConflictList(void) const;
         std::list< CONFLICT_PAIR >*   get_storeConflictList(void) const;
         BOOL                          delete_loadConflictList(void);
         BOOL                          delete_storeConflictList(void);
         BOOL                          push_loadConflictList(CONFLICT_PAIR inPair);
         BOOL                          push_storeConflictList(CONFLICT_PAIR inPair);
         BOOL                          print_loadConflictList(std::ostream &outputStream);
         BOOL                          print_storeConflictList(std::ostream &outputStream);

private:

         // First Order Countable Variables
         UINT_64        numInstructions;                            // Total number of instructions
         UINT_64        numUniqueReads;                             // Total number of Unique Reads
         UINT_64        numUniqueWrites;                            // Total number of Unique Writes
         UINT_64        numSharedReads;                             // Total number of Reads to Shared Memory
         UINT_64        numSharedWrites;                            // Total number of Writes to Shared Memory

         // Second Order Countable Variables
         UINT_64        numMemoryOps;                               // Total number of Reads/Writes
         UINT_64        numIntegerOps;                              // Total number of Integer operations
         UINT_64        numFloatingPointOps;                        // Total number of Floating Point operations

         // Enumerated Values
         CellType       cellType;                                   // Sequential = 0, Transactional = 1
         ConflictType   conflictModel;                              // Conflict Model: Random = 0, High = 1

         // Needed for Body
         UINT_32        basicBlockSize;
         UINT_32        loopCount;

         friend std::ostream& operator<<( std::ostream& os, const Cell& cell );

         //the beginning of the end -- so far as maintainability and performance goes
         std::list< CONFLICT_PAIR > *loadConflictList;
         std::list< CONFLICT_PAIR > *storeConflictList;

};


#endif

