/**
 * @file
 * @author  James Poe   <>, (C) 2008, 2009
 * @date    09/09/08
 * @brief   This is the implementation for the ConstructSkeleton object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: ConstructSkeleton
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include "ConstructSkeleton.h"
#include "Cell.h"
#include "utilities/nanassert.h"
#include <deque>
#include <math.h>

/**
 * @ingroup ConstructSkeleton
 * 
 * @param config 
 */
ConstructSkeleton::ConstructSkeleton ( Config* config )
{
  this->cf = config;
}

/**
 * @ingroup ConstructSkeleton
 *
 * @param 
 */
Skeleton ConstructSkeleton::createSkeleton (  )
{
  std::cout << "Constructing skeleton..." << std::flush;

  /* initialize random seed: */
  srand ( getRDTSC() );

  deque<string> threadNames = cf->readDeque<string>( "calculated", "threadStringDeque" );
  unsigned int numThreads = cf->read<unsigned int>("Global","numThreads");
  I( numThreads == threadNames.size ( ) );
  Skeleton skel ( numThreads );

  for ( unsigned int i = 0; i < numThreads; i++)
  {
    THREAD_CELL_DEQUEP t = skel.getThread( i );
    populateThread ( threadNames[i] , t );
  }

   std::cout << std::endl;
  return skel;
}


/**
 * @ingroup ConstructSkeleton
 * 
 * @param thread 
 * @param tdeq 
 */
void ConstructSkeleton::populateThread ( string thread, THREAD_CELL_DEQUEP tdeq )
{

/*****************************************************************/
/********* GENERIC PORTION ***************************************/
/*****************************************************************/

  unsigned int cellCount = calculateMinimumCellCount ( thread ) ;

  // Create Cells (2x for both transactional and sequantial)
  for (unsigned int x = 0; x < cellCount; x++ )
  {
    tdeq->push_back ( new Cell );
    tdeq->push_back ( new Cell );
  }

  // Create Sequential / Transactional Pointer Deques
  deque < Cell * > seqCells;
  deque < Cell * > transCells;

  // Point to the original Deque
  for ( unsigned int x = 0; x < tdeq->size() ; )
  {
    seqCells.push_back ( tdeq->at ( x++ ) );
    transCells.push_back ( tdeq->at ( x++ ) );
  }

  // Assign Sequential Flags
  for ( unsigned int x = 0; x < seqCells.size() ; x++ )
  {
    seqCells[x]->setCellType ( Sequential );
  }

  // Assign Transactional Flags
  for ( unsigned int x = 0; x < transCells.size() ; x++ )
  {
    transCells[x]->setCellType ( Transactional );
  }


/*****************************************************************/
/********* GRAB OPTION VALUES ************************************/
/*****************************************************************/

  int div;

  deque<long> transInstBucketSizes;
  deque<long> seqInstBucketSizes;
  deque<long> readSetBucketSizes;
  deque<long> writeSetBucketSizes;

  if ( cf->keyExists ( "calculated", "histogramDivisor" ) )
  {
    div = cf->read<int> ( "calculated", "histogramDivisor" );
  }

  if ( cf->keyExists ( "Histogram Settings", "transInstBucketSizes" ) )
  {
    transInstBucketSizes = cf->readDeque<long> ( "Histogram Settings" , "transInstBucketSizes" );
  }

  if ( cf->keyExists ( "Histogram Settings", "seqInstBucketSizes" ) )
  {
    seqInstBucketSizes = cf->readDeque<long> ( "Histogram Settings" , "seqInstBucketSizes" );
  }

  if ( cf->keyExists ( "Histogram Settings", "readSetBucketSizes" ) )
  {
    readSetBucketSizes = cf->readDeque<long> ( "Histogram Settings" , "readSetBucketSizes" );
  }

  if ( cf->keyExists ( "Histogram Settings", "writeSetBucketSizes" ) )
  {
    writeSetBucketSizes = cf->readDeque<long> ( "Histogram Settings" , "writeSetBucketSizes" );
  }

/*****************************************************************/
/********* INSTRUCTION COUNT PORTION *****************************/
/*****************************************************************/

/// *** LIST OPTIONS ***

  // Stride uses seqCells
  if ( cf->isOptionList ( thread, "transStride" ) )
  {
    deque < long > strideValues = cf->readDeque< long > ( thread, "transStride" );
    for ( unsigned int x = 0; x < seqCells.size ( ); x++ )
    {

      // Check to see if we are using the fix for added branch/loop instructions in 
      // Skin stages
      if ( cf->keyExists( "Global" , "seqInstCountFix" ) )
      {
        strideValues [ x ] -= cf->read<int> ( "Global" , "seqInstCountFix" );
        if ( strideValues [ x ] < 0 )
          strideValues [ x ] = 0;
      }

      seqCells[ x ]->setNumInstructions ( strideValues [ x ] );
    }
  }

  // Granularity defines transCells
  if ( cf->isOptionList ( thread, "transGranularity" ) )
  {
    deque < long > granularityValues = cf->readDeque< long > (thread, "transGranularity" );
    for ( unsigned int x = 0; x < transCells.size ( ); x++ )
    {
      transCells[ x ]->setNumInstructions ( granularityValues [ x ] );
    }
  }

/// *** HISTOGRAM OPTIONS ***

  // Stride uses seqCells
  if ( cf->isOptionNormalizedHistogram ( thread, "transStride" ) )
  {
    deque < float > strideHistogram = cf->readDeque< float > ( thread, "transStride" );
    int strideHistogramCounts [ strideHistogram.size() ];

    // Grab counts for each histogram bucket
    for ( unsigned int x = 0; x < strideHistogram.size() ; x++ )
    {
      strideHistogramCounts [ x ] = (int)( roundFloat_b ( ( strideHistogram [ x ] * 100 ) / div ) );
    }

    unsigned int y = 0; // Cell Count


    for ( unsigned int x = 0; x < strideHistogram.size(); x++ )
    {
      while ( strideHistogramCounts [ x ] != 0 )
      {

        // Check to see if we are using the fix for added branch/loop instructions in 
        // Skin stages
        if ( cf->keyExists( "Global" , "seqInstCountFix" ) )
        {
          if ( seqInstBucketSizes [x ] - cf->read<int> ( "Global" , "seqInstCountFix" ) >= 0 )
            seqCells [ y ]->setNumInstructions ( seqInstBucketSizes [ x ] - cf->read<int> ( "Global" , "seqInstCountFix" ) );
          else
            seqCells [ y ]->setNumInstructions ( seqInstBucketSizes [ x ] );
        }

        y++;
        strideHistogramCounts [ x ]--;
      }
    }

    I( y = seqCells.size() );
  }



  // Granularity defines transCells
  if ( cf->isOptionNormalizedHistogram ( thread, "transGranularity" ) )
  {
    deque < float > granularityHistogram = cf->readDeque< float > ( thread, "transGranularity" );
    int granularityHistogramCounts [ granularityHistogram.size() ];

    // Grab counts for each histogram bucket
    for ( unsigned int x = 0; x < granularityHistogram.size() ; x++ )
    {
      granularityHistogramCounts [ x ] = int(( roundFloat_b ( ( granularityHistogram [ x ] * 100.0 ) / div ) ));
//       granularityHistogramCounts [ x ] = int(( roundFloat ( ( granularityHistogram [ x ] * 100.0 ) / div ) ));
// cout << "GranHist (" << granularityHistogram [ x ] << ") * 100: " << (granularityHistogram [ x ] * 100.0 );
// cout << " Rounded:  " << roundFloat(granularityHistogram [ x ] * 100.0 );
// cout << " Floor:  " << int(( floor ( ( granularityHistogram [ x ] * 100.0 ) / div ) ));
// cout << " GranHistCount:  " << granularityHistogramCounts [ x ] << endl;
    }

    unsigned int y = 0; // Cell Count


    for ( unsigned int x = 0; x < granularityHistogram.size(); x++ )
    {
      while ( granularityHistogramCounts [ x ] != 0 )
      {
        transCells [ y ]->setNumInstructions ( transInstBucketSizes [ x ] );
        y++;
        granularityHistogramCounts [ x ]--;
      }
    }

    I( y = transCells.size() );
  }



/*****************************************************************/
/********* CREATE ORDERED LISTS **********************************/
/*****************************************************************/

  Cell *inOrderTransCells[transCells.size()];
  Cell *inOrderSeqCells[seqCells.size()];

  for ( unsigned int x = 0; x < transCells.size(); x++ )
  {
    inOrderTransCells[ x ] = transCells[x];
  }

  qsort ( inOrderTransCells, transCells.size(), sizeof ( Cell * ), instCountComparator );

  for ( unsigned int x = 0; x < seqCells.size(); x++ )
  {
    inOrderSeqCells[ x ] = seqCells[x];
  }

  qsort ( inOrderSeqCells, seqCells.size(), sizeof ( Cell * ), instCountComparator );


/*****************************************************************/
/********* TRANSACTIONAL PORTION - READ/WRITE SETS ***************/
/*****************************************************************/

// *** LIST OPTIONS ***

  if ( cf->isOptionList ( thread, "transReadSetSize" ) )
  {
    deque < long > readSetSizeValues = cf->readDeque< long > (thread, "transReadSetSize" );
    for ( unsigned int x = 0; x < transCells.size ( ); x++ )
    {
      if ( transCells [ x ]->getNumInstructions ( ) >= readSetSizeValues [ x ] )
      {
        transCells[ x ]->setNumUniqueReads ( readSetSizeValues [ x ] );
      }
      else
      {
        cerr << "Fatal Error: Read Set Size larger than Transaction Size\n"
              << "Transaction: " << x << "\nThread: " << thread << endl;
        exit(1);
      }
    }
  }

  if ( cf->isOptionList ( thread, "transWriteSetSize" ) )
  {
    deque < long > writeSetSizeValues = cf->readDeque< long > (thread, "transWriteSetSize" );
    for ( unsigned int x = 0; x < transCells.size ( ); x++ )
    {
      if ( transCells [ x ]->getNumInstructions ( ) >= ( writeSetSizeValues [ x ] + transCells[ x ]->getNumUniqueReads ( ) ))
      {
        transCells[ x ]->setNumUniqueWrites ( writeSetSizeValues [ x ] );
      }
      else
      {
        cerr << "Fatal Error: Write Set Size + Read Set Size larger than Transaction Size\n"
              << "Transaction: " << x << "\nThread: " << thread << endl;
        exit(1);
      }
    }
  }

// *** HISTOGRAM OPTONS ***

  if ( cf->isOptionNormalizedHistogram ( thread, "transReadSetSize" ) )
  {

    deque<float> readSetSizeHistogram = cf->readDeque<float> ( thread, "transReadSetSize" );
    int readSetHistogramCounts [ readSetSizeHistogram.size() ];
    int y = transCells.size() - 1;

    // Grab counts for each histogram bucket
    for ( unsigned int x = 0; x < readSetSizeHistogram.size() ; x++ )
    {
      readSetHistogramCounts [ x ] = (int)( roundFloat_b ( ( readSetSizeHistogram [ x ] * 100 ) / div ) );
    }

    // Iterate through histogram buckets from largest to smallest assigning transactions
    // read sets from largest to smallest
    for ( int x = ( readSetSizeHistogram.size() - 1) ; x >= 0; x-- )
    {
      while ( readSetHistogramCounts [ x ] != 0 )
      {

        if ( inOrderTransCells [ y ]->getNumInstructions ( ) >=  readSetBucketSizes [ x ] )
        {
          inOrderTransCells [ y ]->setNumUniqueReads ( readSetBucketSizes [ x ] );
          readSetHistogramCounts [ x ]--;
        }
        else
        {
          cerr << "Fatal Error: Read Set Size (" << readSetBucketSizes [ x ] << ") larger than Transaction Size (" << inOrderTransCells [ y ]->getNumInstructions ( ) << ")\n"
                << "Transaction: " << y << "\nThread: " << thread << endl;
          exit(1);
        }

        // next smallest transaction
        y--;

      }
    }
  }


  if ( cf->isOptionNormalizedHistogram ( thread, "transWriteSetSize" ) )
  {

    deque<float> writeSetSizeHistogram = cf->readDeque<float> ( thread, "transWriteSetSize" );
    int writeSetHistogramCounts [ writeSetSizeHistogram.size() ];
    int y = transCells.size() - 1;

    // Grab counts for each histogram bucket
    for ( unsigned int x = 0; x < writeSetSizeHistogram.size() ; x++ )
    {
      writeSetHistogramCounts [ x ] = (int)( roundFloat_b ( ( writeSetSizeHistogram [ x ] * 100 ) / div ) );
    }

    int successFlag; 

    // Write set is more complicated because if we can't satisfy writes on this transaction, it is possible 
    // another transaction may be able to satisfy it because of a smaller read set.
    for ( int x = ( writeSetSizeHistogram.size() - 1) ; x >= 0; x-- )
    {
      // While the current bucket still has entries, we will iterate through the list from largest to smallest transaction
      while ( writeSetHistogramCounts [ x ] != 0 )
      {

        int z = x;
        successFlag = false;

        // Look through all of the buckets (starting with the largest) to look for the largest write set that can be 
        // assigned to this transaction
        while ( successFlag == false && ( z >= 0 ) )
        {

          if ( writeSetHistogramCounts [ z ] == 0 )
          {
            successFlag = false;
          }
          else if ( inOrderTransCells [ y ]->getNumInstructions ( ) >=  ( writeSetBucketSizes [ z ] + inOrderTransCells [ y ]->getNumUniqueReads ( ) ) )
          {
            inOrderTransCells [ y ]->setNumUniqueWrites ( writeSetBucketSizes [ z ] );
            writeSetHistogramCounts [ z ]--;
            successFlag = true;
          }

          z--;

        }

        // We were unable to find any write-sets small enough to assign to this transaction, thus we must die
        if ( successFlag == false)
        {
          cerr << "Fatal Error: Unable to find transaction to fit Write Set Size into\n"
                << "Transaction: " << y << "\nThread: " << thread << endl;

          cout << "\n **DEBUG TRACE** " << endl;
          for ( int x = ( transCells.size() - 1 ) ; x >= 0 ; x-- )
            cout << *(inOrderTransCells [ x ]) << endl;
          exit(1);
        }
        else
        {
          y--;
        }

      }
    }
  }

/*****************************************************************/
/********* TRANSACTIONAL PORTION  - SHARED FRQUENCY **************/
/*****************************************************************/

// Populate Memory Options - Note this can only be done after we are guranteed that 
// the unique reads/writes have been assigned

/// *** LIST OPTONS ***

  if ( cf->isOptionList ( thread, "transSharedMemoryFrequency" ) )
  {
    deque < string > sharedMemoryFrequency = cf->readDeque< string > (thread, "transSharedMemoryFrequency" );
    for ( unsigned int x = 0; x < transCells.size ( ); x++ )
    {
      if ( sharedMemoryFrequency [ x ] == "complete" )
      {
        transCells[ x ]->setNumSharedReads ( transCells[ x ]->getNumUniqueReads ( ) );
        transCells[ x ]->setNumSharedWrites ( transCells[ x ]->getNumUniqueWrites ( ) );
      }
      else if ( sharedMemoryFrequency [ x ] == "high" )
      {
        transCells[ x ]->setNumSharedReads ( (UINT_64) roundFloat_b( transCells[ x ]->getNumUniqueReads ( ) * 0.75 ) );
        transCells[ x ]->setNumSharedWrites ( (UINT_64) roundFloat_b( transCells[ x ]->getNumUniqueWrites ( ) * 0.75 ) );
      }
      else if ( sharedMemoryFrequency [ x ] == "low" )
      {
        transCells[ x ]->setNumSharedReads ( (UINT_64) roundFloat_b( transCells[ x ]->getNumUniqueReads ( ) * 0.25 ) );
        transCells[ x ]->setNumSharedWrites ( (UINT_64) roundFloat_b( transCells[ x ]->getNumUniqueWrites ( ) * 0.25 ) );
      }
      else if ( sharedMemoryFrequency [ x ] == "minimal" )
      {
        if ( (transCells[ x ]->getNumUniqueReads ( ) < 1 ) && (transCells[ x ]->getNumUniqueWrites ( ) < 1 ) )
        {
          cerr << "Fatal Error: Requested Minimumal Shared but Transaction has 0 reads/writes\n"
              << "Transaction: " << x << "\nThread: " << thread << endl;
          exit(1);
        }
        else
        {
          transCells[ x ]->setNumSharedReads ( 1 );
          transCells[ x ]->setNumSharedWrites ( 1 );
        }
      }
      else if ( sharedMemoryFrequency [ x ] == "none" )
      {
        transCells[ x ]->setNumSharedReads ( 0 );
        transCells[ x ]->setNumSharedWrites ( 0 );
      }
      else
      {
        cerr << "Fatal Error: Unrecognized transSharedMemoryFrequency input\n"
            << "Transaction: " << x << "\nThread: " << thread << endl;
        exit(1);
      }
    }
  }


/// *** HISTOGRAM OPTONS ***

  if ( cf->isOptionNormalizedHistogram ( thread, "transSharedMemoryFrequency" ) )
  {

    deque<float> sharedMemoryHistogram = cf->readDeque<float> ( thread, "transSharedMemoryFrequency" );
    int sharedMemoryHistogramCounts [ sharedMemoryHistogram.size() ];

    // Grab counts for each histogram bucket
    for ( unsigned int x = 0; x < sharedMemoryHistogram.size() ; x++ )
    {
      sharedMemoryHistogramCounts [ x ] = (int)( roundFloat_b ( ( sharedMemoryHistogram [ x ] * 100 ) / div ) );
    }

    int sharedResults;

    int debugCheck = 0;
    for ( unsigned int x = 0; x < sharedMemoryHistogram.size() ; x++)
      debugCheck += sharedMemoryHistogramCounts [ x ];
    I( debugCheck == transCells.size ( ) );

    for ( unsigned int x = 0; x < transCells.size ( ); x++ )
    {

      sharedResults = getRandomNumber ( 0 , sharedMemoryHistogram.size() );
      while ( sharedMemoryHistogramCounts [ sharedResults ] == 0 )
      {
        sharedResults = getRandomNumber ( 0 , sharedMemoryHistogram.size() );
      }

      sharedMemoryHistogramCounts [ sharedResults ]--;

      switch ( sharedResults )
      {
        // Complete
        case 0:
          transCells[ x ]->setNumSharedReads ( transCells[ x ]->getNumUniqueReads ( ) );
          transCells[ x ]->setNumSharedWrites ( transCells[ x ]->getNumUniqueWrites ( ) );
          break;
        // High
        case 1:
          transCells[ x ]->setNumSharedReads ( (UINT_64) roundFloat_b( transCells[ x ]->getNumUniqueReads ( ) * 0.75 ) );
          transCells[ x ]->setNumSharedWrites ( (UINT_64) roundFloat_b( transCells[ x ]->getNumUniqueWrites ( ) * 0.75 ) );
          break;
        // Low
        case 2:
          transCells[ x ]->setNumSharedReads ( (UINT_64) roundFloat_b( transCells[ x ]->getNumUniqueReads ( ) * 0.25 ) );
          transCells[ x ]->setNumSharedWrites ( (UINT_64) roundFloat_b( transCells[ x ]->getNumUniqueWrites ( ) * 0.25 ) );
          break;
        // Minimal
        case 3:
          if ( transCells[ x ]->getNumUniqueReads ( ) >= 1 )
          {
            transCells[ x ]->setNumSharedReads ( 1 );
          }
          if ( transCells[ x ]->getNumUniqueWrites ( ) >= 1 )
          {
            transCells[ x ]->setNumSharedWrites ( 1 );
          }
          break;
        // None
        case 4:
          transCells[ x ]->setNumSharedReads ( 0 );
          transCells[ x ]->setNumSharedWrites ( 0 );
          break;
      }
    }
  }

/*****************************************************************/
/********* TRANSACTIONAL PORTION - CONFLICT DISTRIBUTION *********/
/*****************************************************************/

/// *** LIST OPTONS ***

  if ( cf->isOptionList ( thread, "transConflictDistributionModel" ) )
  {
    deque < string > conflictDistModel = cf->readDeque< string > (thread, "transConflictDistributionModel" );
    for ( unsigned int x = 0; x < transCells.size ( ); x++ )
    {
      if ( conflictDistModel [ x ] == "high" )
      {
        transCells[ x ]->setConflictModel( High );
      }
      else if ( conflictDistModel [ x ] == "random" )
      {
        transCells[ x ]->setConflictModel( Random );
      }
      else
      {
          cerr << "Fatal Error: Unrecognized transConflictDistributionModel input\n"
              << "Transaction: " << x << "\nThread: " << thread << endl;
          exit(1);
      }
    }
  }

/// *** HISTOGRAM OPTONS ***

  if ( cf->isOptionNormalizedHistogram ( thread, "transConflictDistributionModel" ) )
  {

    deque<float> conflictDistHistogram = cf->readDeque<float> ( thread, "transConflictDistributionModel" );
    int conflictDistHistogramCounts [ conflictDistHistogram.size() ];

    // Grab counts for each histogram bucket
    for ( unsigned int x = 0; x < conflictDistHistogram.size() ; x++ )
    {
      conflictDistHistogramCounts [ x ] = (int)( roundFloat_b ( ( conflictDistHistogram [ x ] * 100 ) / div ) );
    }

    int conflictResults;

    int debugCheck = 0;
    for ( unsigned int x = 0; x < conflictDistHistogram.size() ; x++)
      debugCheck += conflictDistHistogramCounts [ x ];
    I( debugCheck == transCells.size ( ) );

    for ( unsigned int x = 0; x < transCells.size ( ); x++ )
    {

      conflictResults = getRandomNumber ( 0 , conflictDistHistogram.size() );
      while ( conflictDistHistogramCounts [ conflictResults ] == 0 )
      {
        conflictResults = getRandomNumber ( 0 , conflictDistHistogram.size() );
      }

      conflictDistHistogramCounts [ conflictResults ]--;

      switch ( conflictResults )
      {
        case 0:
          transCells[ x ]->setConflictModel( High );
          break;
        case 1:
          transCells[ x ]->setConflictModel( Random );
          break;
      }
    }
  }



/*****************************************************************/
/********* TRANSACTIONAL PORTION - INSTRUCTION MIX ***************/
/*****************************************************************/

// Populate Instruction Mixes - MUST BE VERY LAST
  // Check to see if it is a normalized histogram (note, it has to be!)
  if ( cf->isOptionNormalizedHistogram ( thread, "transInstructionMix" ) )
  {

    deque < float > transInstructionMix = cf->readDeque< float > (thread, "transInstructionMix" );
    float memOpDist = transInstructionMix [ 0 ];
    float intOpDist = transInstructionMix [ 1 ];
    float fpOpDist = transInstructionMix [ 2 ];
    int memOps = 0, intOps = 0, fpOps = 0, totalInsts = 0, remainingInsts = 0;

    for ( unsigned int x = 0; x < transCells.size ( ); x++ )
    {

      totalInsts = transCells[ x ]->getNumInstructions ( );
      memOps = transCells[ x ]->getNumUniqueReads ( ) + transCells[ x ]->getNumUniqueWrites ( );

      // If we have not used all of the MemOps necessary creating Unique ops
      if ( ( roundFloat_b ( memOpDist * totalInsts ) ) >= memOps )
      {

        memOps = (int) roundFloat_b ( memOpDist * totalInsts );
        intOps = (int) roundFloat_b ( intOpDist * totalInsts );
        fpOps = (int) roundFloat_b ( fpOpDist * totalInsts );

        // This will assign any leftover due to roundFloat_b randomly
        switch ( getRandomNumber ( 1 , 3 ) )
        {
          case 1: memOps += totalInsts - (memOps + intOps + fpOps); break;
          case 2: intOps += totalInsts - (memOps + intOps + fpOps); break;
          case 3: fpOps += totalInsts - (memOps + intOps + fpOps); break;
          default: cerr << "Fatal Error: Randomization Function Error!" << endl; exit(1); break;
        }

      }
      // We have already surpassed our memOp mix creating the unique reads/writes
      else
      {

        memOps = memOps;
        remainingInsts = totalInsts - memOps;

        if ( getRandomNumber ( 1, 2) == 1 )
        {
          intOps = (int) roundFloat_b ( ( 1.0 /  ( (intOpDist + fpOpDist ) / intOpDist ) ) * remainingInsts );
          fpOps = remainingInsts - intOps;
        }
        else
        {
          fpOps = (int) roundFloat_b ( ( 1.0 /  ( (intOpDist + fpOpDist ) / fpOpDist ) ) * remainingInsts );
          intOps = remainingInsts - fpOps;
        }

      }

      transCells[ x ]->setNumMemoryOps ( memOps );
      transCells[ x ]->setNumIntegerOps ( intOps );
      transCells[ x ]->setNumFloatingPointOps ( fpOps );
    }

  }
  else
  {
      cerr << "Fatal Error: Non normalized histogram given to transInstructionMix\n"
          <<  "\nThread: " << thread << endl;
      exit(1);
  }


/*****************************************************************/
/*************** SEQUENTIAL PORTION ******************************/
/*****************************************************************/

  // Handle all of the sequential portions
  // note, these are just given mostly generic values now
  if ( cf->isOptionNormalizedHistogram ( thread, "sequentialInstructionMix" ) )
  {
    deque < float > sequentialInstructionMix = cf->readDeque< float > (thread, "transInstructionMix" );
    float seqMemOpDist = sequentialInstructionMix [ 0 ];
    float seqIntOpDist = sequentialInstructionMix [ 1 ];
    float seqFpOpDist = sequentialInstructionMix [ 2 ];
    int seqMemOps = 0, seqIntOps = 0, seqFpOps = 0, seqTotalInsts = 0, seqRemainingInsts = 0;

    for ( unsigned int x = 0; x < seqCells.size ( ); x++ )
    {
      seqTotalInsts = seqCells[ x ]->getNumInstructions ( );

      seqMemOps = (int) roundFloat_b ( seqMemOpDist * seqTotalInsts );
      seqIntOps = (int) roundFloat_b ( seqIntOpDist * seqTotalInsts );
      seqFpOps = (int) roundFloat_b ( seqFpOpDist * seqTotalInsts );

      // This will assign any leftover due to roundFloat_b randomly
      switch ( getRandomNumber ( 1 , 3 ) )
      {
        case 1: seqMemOps += seqTotalInsts - (seqMemOps + seqIntOps + seqFpOps); break;
        case 2: seqIntOps += seqTotalInsts - (seqMemOps + seqIntOps + seqFpOps); break;
        case 3: seqFpOps  += seqTotalInsts - (seqMemOps + seqIntOps + seqFpOps); break;
        default: cerr << "Fatal Error: Randomization Function Error!" << endl; exit(1); break;
      }

      // Assign all of the values.  Note that Unique Read/Writes is just a 
      //  ratio of the total number of memory operations for the moment.
      seqCells[ x ]->setNumMemoryOps ( seqMemOps );
      seqCells[ x ]->setNumIntegerOps ( seqIntOps );
      seqCells[ x ]->setNumFloatingPointOps ( seqFpOps );

      if(seqMemOps > 65536)
      {
         seqCells[ x ]->setNumUniqueReads ( (int)  roundFloat_b ( ( seqMemOps * 0.0003 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.000105 ) + 1 ) );
      }
      else if(seqMemOps > 32768)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.0004 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.00014 ) + 1 ) );
      }
      else if(seqMemOps > 16384)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.0005 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.000175 ) + 1 ) );
      }
      else if(seqMemOps > 8192)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.001 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.00035 ) + 1 ) );
      }
      else if(seqMemOps > 4096)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.004 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.0014 ) + 1 ) );
      }
      else if(seqMemOps > 2048)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.01 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.0035 ) + 1 ) );
      }
      else if(seqMemOps > 1024)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.05 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.0175 ) + 1 ) );
      }
      else if(seqMemOps > 512)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.10 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.035 ) + 1 ) );
      }
      else if(seqMemOps > 256)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.14 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.049 ) + 1 ) );
      }
      else if(seqMemOps > 128)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.20 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.07 ) + 1 ) );
      }
      else if(seqMemOps > 64)
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.24 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.08 ) + 1 ) );
      }
      else
      {
         seqCells[ x ]->setNumUniqueReads ( (int) roundFloat_b ( ( seqMemOps * 0.30 ) + 1 ) );
         seqCells[ x ]->setNumUniqueWrites ( (int) roundFloat_b ( ( seqMemOps * 0.10 ) + 1 ) );
      }

      seqCells[ x ]->setNumSharedReads ( 0 );
      seqCells[ x ]->setNumSharedWrites ( 0 );
      seqCells[ x ]->setConflictModel ( Random );
      }
  }
  else
  {
    cerr << "Fatal Error: Non normalized histogram given to sequentialInstructionMix\n"
        <<  "\nThread: " << thread << endl;
    exit(1);
  }
}





/**
 * @ingroup ConstructSkeleton
 * 
 * @param thread 
 * @return unsigned int
 */
unsigned int ConstructSkeleton::calculateMinimumCellCount ( string thread )
{
  // Begin cellCount at Minimum Value
  unsigned int cellCount = 0;

  deque<string> mandatoryThreadOptions = cf->readDeque< string > ( "calculated" , "mandatoryThreadOptions" );

  // First round tests to see if there are any lists, and if so, find the number of cells needed
  // to satisfy the list and verify that all lists are the same size
  for ( unsigned int x = 0; x < mandatoryThreadOptions.size( ); x++ )
  {
    if ( cf->isOptionList( thread, mandatoryThreadOptions[x] ) )
    {
      if ( cellCount == 0 )
      {
        cellCount = cf->readDeque<long>( thread , mandatoryThreadOptions[x] ).size();
      }
      else 
      {
        if (cellCount != cf->readDeque<long>( thread , mandatoryThreadOptions[x] ).size() )
        {
          cerr << "Fatal Error: Multiple lists of different sizes given in options for: "<< thread << endl;
          exit(1);
        }
      }
    }
  }

  // If we have no lists, determine the cellCount from the histograms
  if ( cellCount == 0 )
  {

    list<long> histogramValues;

    // Go through all of the histograms and grab the values
    for ( unsigned int x = 0; x < mandatoryThreadOptions.size( ); x++ )
    {
      if ( cf->isOptionNormalizedHistogram( thread, mandatoryThreadOptions[x] ) )
      {
        deque<float> temp = cf->readDeque<float>( thread, mandatoryThreadOptions[x] );
        for ( unsigned int x = 0; x < temp.size(); x++ )
        {
          if ( temp[x] != 0 )
            histogramValues.push_back ( (long) ( roundFloat_b ( temp[x] * 100 ) ) );
        }
      }
    }

    // Calculate the greatest common divisor of all of the histograms
    long div = getGreatestCommonDivisor ( histogramValues );

    // Write the histogram divisor to the config file
    cf->add<long> ( "calculated" , "histogramDivisor" , div );

    // The minimum cell count is 100 divided by the greatest common divisor
    cellCount = (int) roundFloat_b ( 100.0 / div ) ;

  }

  return cellCount;
}

