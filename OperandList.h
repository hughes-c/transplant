/**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009
 * @date    09/19/08
 * @brief   Storage class used to store register values for instructions.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: Operandlist
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef OPERANDLIST_H
#define OPERANDLIST_H

#include <string>

#include "param_types.h"

/**
 * @ingroup Instruction
 * @brief   
 */
class OperandList
{
   public:
      OperandList(): rs(""),rt(""),rd(""),imm(""),rs_variable(""),rt_variable(""),rd_variable(""), clobberList(""), offSet(0) {}

      std::string rs;
      std::string rt;
      std::string rd;
      std::string imm;

      std::string rs_variable;
      std::string rt_variable;
      std::string rd_variable;

      std::string clobberList;

      UINT_32     offSet;
};

#endif
