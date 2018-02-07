/**
 * @file
 * @author  Clay Hughes & James Poe  <>, (C) 2008, 2009
 * @date    09/19/08
 * @brief   Types and enumerations for TransPlant
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PARAM_TYPES_H
#define PARAM_TYPES_H

#include <stdint.h>

//NOTE DO NOT CHANGE!
#define _16_BIT_RANGE 65535
#define _15_BIT_RANGE 32767

//standard
typedef bool BOOL;

typedef uint8_t  UINT_8;
typedef uint16_t UINT_16;
typedef uint32_t UINT_32;
typedef uint64_t UINT_64;

typedef int8_t  INT_8;
typedef int16_t INT_16;
typedef int32_t INT_32;
typedef int64_t INT_64;

typedef uint32_t ADDRESS_INT;
typedef uint32_t THREAD_ID;
typedef uint32_t TX_ID;

typedef int32_t  IntRegValue;

//cross
typedef std::pair<ADDRESS_INT, UINT_32> CONFLICT_PAIR;               //specifies address-offset for use in read- and write-set conflicts

enum CellType
{
   Sequential,
   Transactional,
   LoopStart,
   LoopEnd,
   Synchronization
};

enum ConflictType
{
   Random,
   High,
   Specified,
   None
};

enum RegType
{
   RegInvalid = 0,
   T2 = 10,
   T3,
   T4,
   T5,
   T6,
   T7,
   S0,
   S1,
   S2,
   S3,
   S4,
   S5,
   FP2  = 35,
   FP4  = 37,
   FP6  = 39,
   FP8  = 41,
   FP10 = 43,
   FP12 = 45,
   FP14 = 47,
   FP16 = 49,
   FP18 = 51,
   FP20 = 53,
};

//enumerations ganked from SESC
enum InstType
{
   iOpInvalid = 0,     //!< Never used.
   iALU,               //!< Simple int ALU operations (+,-,>>)
   iMult,              //!< Integer Multiplication
   iDiv,               //!< Integer Division
   iBJ,                //!< branches, calls, and jumps
   iLoad,              //!< Load
   iStore,             //!< Store
   fpALU,              //!< coprocessor 1 floating point adds, converts, etc
   fpMult,             //!< floating point multiplies
   fpDiv,              //!< floating point division and sqrt
   iBeginTX,
   iCommitTX,
   iInitLoop,
   iBeginLoop,
   iEndLoop,
   syncBarrier,
   MaxInstType
};

enum InstSubType
{
   iSubInvalid = 0,
   iNop,          //! Nop
   iMemory,       //! Load & Store
   BJUncond,      //! iBJ opcode
   BJCall,        //! iBJ opcode
   BJRet,         //! iBJ opcode
   BJCond,        //! iBJ opcode
   MultiCellLoop, //! Multi-Inner-Cell Loop
   CellLoop,      //! Inner-Cell Loop
   BlockLoop,     //! Multi-Cell Loop
   InstSubTypeMax
};

#endif
