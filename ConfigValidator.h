/**
 * @file
 * @author  James Poe   <>, (C) 2008, 2009
 * @date    09/08/08
 * @brief   This is the interface for the ConfigValidator object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Interface: ConfigValidator
 * 
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONFIGVALIDATOR_H
#define CONFIGVALIDATOR_H

#include "Config.h"
#include <deque>

/**
 * @ingroup ConfigValidator
 * @brief   Helper class for Configuration
 *
 */
class ConfigValidator {


public:

          // This class will be used statically - so constructor doesnt' do much
          ConfigValidator();

          // Static function used to validate the options from an input file
          static bool validate ( Config* cf );

private:
          // Called by validate to confirm all required options are available
          static bool confirmRequiredOptionsPresent ( Config* cf );
          // Helper function called to check that individual thread options are present
          static bool checkThreadOptionsPresent ( Config* cf , const string& thread );
};

#endif
