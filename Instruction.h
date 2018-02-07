 /**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009, 2010
 * @date    09/19/08
 * @brief   This is the interface for the Instruction object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: Instruction
 * Container used to replicate a MIPS-type instruction.
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
 
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <sstream>

#include "OperandList.h"
#include "param_types.h"

/**
 * @ingroup Instruction
 * @brief   Instruction container class
 *
 * This class contains the methods and fields required for storing
 * a MIPS instruction.
 */
class Instruction
{
   public:
      Instruction();
      Instruction(InstType opCodeIn);
      Instruction(const Instruction& objectIn);
      ~Instruction();

      BOOL           set_opCode(InstType opCode);
      BOOL           set_subCode(InstSubType subCode);
      BOOL           set_rs(RegType rs);
      BOOL           set_rt(RegType rt);
      BOOL           set_rd(RegType rd);
      BOOL           set_immediate(INT_64 immediate);
      BOOL           set_physicalAddress(ADDRESS_INT physicalAddress);
      BOOL           set_virtualAddress(ADDRESS_INT virtualAddress);
      BOOL           set_isShared(BOOL isShared);
      BOOL           set_isUnique(BOOL isUnique);
      BOOL           set_matchedInstruction(INT_32 matched);
      BOOL           set_iterations(UINT_32 iterations);
      BOOL           set_instructionID(UINT_32 instructionID);
      BOOL           set_conflictModel(ConflictType conflictModel);

      InstType       get_opCode(void);
      InstSubType    get_subCode(void);
      RegType        get_rs(void);
      RegType        get_rt(void);
      RegType        get_rd(void);
      INT_64         get_immediate(void);
      ADDRESS_INT    get_virtualAddress(void);
      ADDRESS_INT    get_physicalAddress(void);
      BOOL           get_isShared(void);
      BOOL           get_isUnique(void);
      INT_32         get_matchedInstruction(void);
      UINT_32        get_iterations(void);
      UINT_32        get_instructionID(void);
      ConflictType   get_conflictModel(void) const;

      static RegType       identifyRegister(INT_32 registerNum);
      static std::string   getIntVariable(RegType registerIn);
      static std::string   getFPVariable(RegType registerIn);
      static std::string   IntToString(INT_64 input);

   protected:


   private:
      /* Variables */
      InstType             opCode;
      InstSubType          subCode;
      RegType              rs;
      RegType              rt;
      RegType              rd;
      INT_64               immediate;
      ADDRESS_INT          physicalAddress;
      ADDRESS_INT          virtualAddress;

      BOOL                 isShared;
      BOOL                 isUnique;
      INT_32               matched;
      UINT_32              iterations;
      UINT_32              instructionID;

      ConflictType         conflictModel;
};

#endif
