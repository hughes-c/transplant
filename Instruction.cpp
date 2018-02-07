/**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009, 2010
 * @date    09/19/08
 * @brief   This is the implementation for the Instruction container.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: Instruction
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include "Instruction.h"

/**
 * @ingroup Instruction
 * @brief Default constructor
 */
Instruction::Instruction()
{
   opCode  = iOpInvalid;
   subCode = iSubInvalid;
   rs = RegInvalid;
   rt = RegInvalid;
   rd = RegInvalid;
   immediate = 0;
   physicalAddress = 0;
   virtualAddress  = 0;

   isShared = 0;
   isUnique = 0;

   matched = -1;
   iterations = 0;
   instructionID = 0;

   conflictModel = Random;
}

/**
 * @ingroup Instruction
 * @brief Constructor
 */
Instruction::Instruction(InstType opCodeIn) : opCode(opCodeIn)
{
   if(opCodeIn == iLoad || opCodeIn == iStore)
      subCode = iMemory;
   else
      subCode = iSubInvalid;

   rs = RegInvalid;
   rt = RegInvalid;
   rd = RegInvalid;
   immediate = 0;
   physicalAddress = 0;
   virtualAddress  = 0;

   isShared = 0;
   isUnique = 0;

   matched = -1;
   iterations = 0;

   conflictModel = Random;
}

/**
 * @ingroup Instruction
 * @brief Copy constructor
 */
Instruction::Instruction(const Instruction& objectIn)
{
   opCode  = objectIn.opCode;
   subCode = objectIn.subCode;

   rs = objectIn.rs;
   rt = objectIn.rt;
   rd = objectIn.rd;
   immediate = objectIn.immediate;
   physicalAddress = objectIn.physicalAddress;
   virtualAddress  = objectIn.virtualAddress;

   isShared = objectIn.isShared;
   isUnique = objectIn.isUnique;

   matched = objectIn.matched;
   iterations = objectIn.iterations;

   conflictModel = objectIn.conflictModel;
}

/**
 * @ingroup Instruction
 * @brief Default destructor
 */
Instruction::~Instruction()
{
}

BOOL Instruction::set_opCode(InstType opCode)
{
   this->opCode = opCode;
   return 1;
}

BOOL Instruction::set_subCode(InstSubType subCode)
{
   this->subCode = subCode;
   return 1;
}

BOOL Instruction::set_rs(RegType rs)
{
   this->rs = rs;
   return 1;
}

BOOL Instruction::set_rt(RegType rt)
{
   this->rt = rt;
   return 1;
}

BOOL Instruction::set_rd(RegType rd)
{
   this->rd = rd;
   return 1;
}

BOOL Instruction::set_immediate(INT_64 immediate)
{
   this->immediate = immediate;
   return 1;
}

BOOL Instruction::set_physicalAddress(ADDRESS_INT physicalAddress)
{
   this->physicalAddress = physicalAddress;
   return 1;
}

BOOL Instruction::set_virtualAddress(ADDRESS_INT virtualAddress)
{
   this->virtualAddress = virtualAddress;
   return 1;
}

BOOL Instruction::set_isShared(BOOL isShared)
{
   this->isShared = isShared;
   return 1;
}

BOOL Instruction::set_isUnique(BOOL isUnique)
{
   this->isUnique = isUnique;
   return 1;
}

BOOL Instruction::set_matchedInstruction(INT_32 matched)
{
   this->matched = matched;
   return 1;
}

BOOL Instruction::set_iterations(UINT_32 iterations)
{
   this->iterations = iterations;
   return 1;
}

BOOL Instruction::set_instructionID(UINT_32 instructionID)
{
   this->instructionID = instructionID;
   return 1;
}

BOOL Instruction::set_conflictModel(ConflictType conflictModel)
{
   this->conflictModel = conflictModel;
   return 1;
}

InstType Instruction::get_opCode()
{
   return this->opCode;
}

InstSubType Instruction::get_subCode()
{
   return this->subCode;
}

RegType Instruction::get_rs()
{
   return this->rs;
}

RegType Instruction::get_rt()
{
   return this->rt;
}

RegType Instruction::get_rd()
{
   return this->rd;
}

INT_64 Instruction::get_immediate()
{
   return this->immediate;
}

ADDRESS_INT Instruction::get_virtualAddress()
{
   return this->virtualAddress;
}

ADDRESS_INT Instruction::get_physicalAddress()
{
   return this->physicalAddress;
}

BOOL Instruction::get_isShared()
{
   return this->isShared;
}

BOOL Instruction::get_isUnique()
{
   return this->isUnique;
}

INT_32 Instruction::get_matchedInstruction()
{
   return this->matched;
}

UINT_32 Instruction::get_iterations()
{
   return this->iterations;
}

UINT_32 Instruction::get_instructionID()
{
   return this->instructionID;
}

std::string Instruction::IntToString(INT_64 input)
{
  std::ostringstream output;
  output << input;

  return output.str();
}

ConflictType Instruction::get_conflictModel(void) const
{
   return conflictModel;
}

//only a subset of the registers are used, the rest are
//reserved for variables required to maintain state
//information in the synthetic
RegType Instruction::identifyRegister(INT_32 registerNum)
{
   /* Variables */

   /* Processes */
   if(registerNum < 35)
   {
      switch(registerNum)
      {
         case 10:
            return T2;
         case 11:
            return T3;
         case 12:
            return T4;
         case 13:
            return T5;
         case 14:
            return T6;
         case 15:
            return T7;
         case 16:
            return S0;
         case 17:
            return S1;
         case 18:
            return S2;
         case 19:
            return S3;
         case 20:
            return S4;
         case 21:
            return S5;
      }
   }
   else
   {
      switch(registerNum)
      {
         case 35:
         case 36:
            return FP2;
         case 37:
         case 38:
            return FP4;
         case 39:
         case 40:
            return FP6;
         case 41:
         case 42:
            return FP8;
         case 43:
         case 44:
            return FP10;
         case 45:
         case 46:
            return FP12;
         case 47:
         case 48:
            return FP14;
         case 49:
         case 50:
            return FP16;
         case 51:
         case 52:
            return FP18;
         case 53:
         case 54:
            return FP20;
      }
   }

}


std::string Instruction::getIntVariable(RegType registerIn)
{
   /* Variables */
   std::string variableList;

   /* Processes */
   if(registerIn == 99)
   {
      variableList = "";
   }
   else
   {
      //saved temporary 20-23 used for memory accesses
      switch(registerIn)
      {
         case 0:
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         case 6:
         case 7:
         case 8:
            variableList = "r\"(r_out_t0) ";
            break;
         case 9:
            variableList = "r\"(r_out_t1) ";
            break;
         case 10:
            variableList = "r\"(r_out_t2) ";
            break;
         case 11:
            variableList = "r\"(r_out_t3) ";
            break;
         case 12:
            variableList = "r\"(r_out_t4) ";
            break;
         case 13:
         case 14:
         case 15:
            variableList = "r\"(r_out_t5) ";
            break;
         case 16:
            variableList = "r\"(r_out_s0) ";
            break;
         case 17:
            variableList = "r\"(r_out_s1) ";
            break;
         case 18:
            variableList = "r\"(r_out_s2) ";
            break;
         case 19:
            variableList = "r\"(r_out_s3) ";
            break;
         case 20:
            variableList = "r\"(r_out_t4) ";
            break;
         case 21:
         case 22:
         case 23:
            variableList = "r\"(r_out_t5) ";
            break;
         case 24:
         case 25:
         case 26:
         case 27:
         case 28:
         case 29:
         case 30:
         case 31:
         default:
            variableList = "r\"(r_out_s0) ";
            break;
      }
   }

   return variableList;
}

std::string Instruction::getFPVariable(RegType registerIn)
{
   /* Variables */
   std::string variableList;

   /* Processes */
   if(registerIn == 99)
   {
      variableList = "";
   }
   else
   {
      switch(registerIn)
      {
         case 35:
            variableList = "f\"(r_out_f2) ";
            break;
         case 37:
            variableList = "f\"(r_out_f4) ";
            break;
         case 39:
            variableList = "f\"(r_out_f6) ";
            break;
         case 41:
            variableList = "f\"(r_out_f8) ";
            break;
         case 43:
            variableList = "f\"(r_out_f10)";
            break;
         case 45:
         case 47:
         case 49:
         case 51:
         case 53:
         case 55:
         case 57:
         case 59:
         case 61:
         case 63:
         default:
            variableList = "f\"(r_out_f12)";
            break;
      }
   }

   return variableList;
}
