/**
 * @file
 * @author  James Poe   <>, (C) 2008, 2009
 * @date    09/08/08
 * @brief   This is the implementation for the ConfigValidator object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: ConfigValidator
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


#include "ConfigValidator.h"


ConfigValidator::ConfigValidator ( )
{
}



/**
 * @ingroup ConfigValidator 
 *
 * @param config*
 * @return bool
 *
 *  This is the static function called that will orchestrate all of 
 *  the various validation tests performed on the configuration 
 *  supplied by a user.
 */
bool ConfigValidator::validate ( Config* cf )
{
  std::cout << "Validating configuration..." << std::endl;

  //  STAGE 1: Check to ensure all necessary parameters are passed!
  if ( !confirmRequiredOptionsPresent ( cf ) )
    return false;

  return true;
}



/**
 * @ingroup ConfigValidator 
 *
 * @param config*
 * @return bool
 *
 *  This function performs a stage 1 check; namely that all 
 *  configuration options that are required are at a minimum 
 *  present in the configuration object.
 */
bool ConfigValidator::confirmRequiredOptionsPresent ( Config* cf )
{

  bool retVal = true;

  // Check for Global Options

  if( !cf->keyExists ( "Global" , "numThreads" ) )
  {
    cerr << "Fatal Error: Optional \"Global->numThreads\" not found!" << endl;
    return false;
  }

  if( !cf->keyExists ( "Global" , "homogeneous" ) )
  {
    cerr << "Fatal Error: Optional \"Global->homogeneous\" not found!" << endl;
    return false;
  }

  //We don't really care if barriers exist or not, this just ensures that the later
  //stages can all assume that the key exists even if it is zero.
  if( !cf->keyExists ( "Global" , "numBarriers" ) )
  {
    cf->add<int>( "Global" , "numBarriers", 0 );
  }

  // We now know both the number of threads, as well whether they are homogeneous.
  // Check for individual thread options.

  int numThreads = cf->read<int> ( "Global" , "numThreads"); 

  // Note, threadNames is used so we don't have to build them again later
  deque < string > threadNames;


  for ( int z = 0 ; z < numThreads ; z++ )
  {
    std::ostringstream o;

    if ( cf->read<bool> ( "Global" , "homogeneous" ) )
    {
      o << "Thread0";
    }
    else
    {
      o << "Thread" << z;
    }

    retVal = retVal && checkThreadOptionsPresent ( cf , o.str() );
    threadNames.push_back( o.str( ) );
  }


  cf->addDeque( "calculated" , "threadStringDeque" , threadNames );

  return retVal;
}



/**
 * @ingroup ConfigValidator 
 *
 * @param config*
 * @param string&
 * @return bool
 *
 *  This is a help function that when passed the thread name,
 *  will ensure that all of the individual thread required options
 *  are present in the configuration object.
 */
bool ConfigValidator::checkThreadOptionsPresent ( Config* cf , const string& thread )
{
  bool retVal = true;

  deque < string > mandatoryThreadOptions;

  mandatoryThreadOptions.push_back("transGranularity");
  mandatoryThreadOptions.push_back("transStride");
  mandatoryThreadOptions.push_back("transReadSetSize");
  mandatoryThreadOptions.push_back("transWriteSetSize");
  mandatoryThreadOptions.push_back("transSharedMemoryFrequency");
  mandatoryThreadOptions.push_back("transConflictDistributionModel");

  cf->addDeque ( "calculated" , "mandatoryThreadOptions" , mandatoryThreadOptions );

  for ( unsigned int x = 0; x < mandatoryThreadOptions.size ( ); x++ )
  {
    if( !cf->keyExists ( thread , mandatoryThreadOptions[ x ] ) )
    {
      cerr << "Fatal Error: Option \""<< thread << "->" << mandatoryThreadOptions[x] << "\" not found!" << endl;
      retVal = false;
    }
  }

  return retVal;
}
