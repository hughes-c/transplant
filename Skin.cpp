/**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009, 2010
 * @date    09/19/08
 * @brief   This is the implementation for the Skin object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: Skin
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include "Skin.h"

/**
 * @ingroup Skin
 * @brief Default constructor
 */
Skin::Skin() : matchID(0), Skeleton()
{
}

/**
 * @ingroup Skin
 * @brief Constructor
 */
Skin::Skin(Skeleton &skeletonIn) : matchID(0), Skeleton(skeletonIn)
{
}

/**
 * @ingroup Skin
 * @brief Default destructor
 */
Skin::~Skin()
{
//    for(std::deque< Instruction * >::iterator instructionList_it = instructionList.begin(); instructionList_it != instructionList.end(); instructionList_it++)
//    {
//       delete (*instructionList_it);
//    }
}

UINT_32 Skin::updateConfig(Config *config)
{
   this->config = config;
   return 1;
}

/**
 * @ingroup Skin
 * 
 * @note Converts the cells into basic block-type structures and populates the instructions
 * @param
 *
 * Builds instruction lists for each cell within the Skeleton and links them together.
 */
void Skin::spinalColumn(void)
{
   /* Variables */
   BOOL const randomizeStream = 1;                                //randomize the instruction stream?

   UINT_32  blockLoop;
   UINT_32  blockLoopCount;

   UINT_64  numMemOps;
   UINT_64  numFloatOps;
   UINT_64  numIntegerOps;

   UINT_64  numLoadOps;
   UINT_64  numStoreOps;

   UINT_64  sharedReads, uniqueReads;
   UINT_64  sharedWrites, uniqueWrites;

   THREAD_CELL_DEQUEP cellList;
   Instruction *instruction;

   /* Processes */
   std::cout << "Assembling spine for thread ";
   for(UINT_32 threadID = 0; threadID < numThreads; threadID++)
   {
      std::cout << threadID << "..." << std::flush;

      std::deque< Instruction * > *instructionList = new std::deque< Instruction * > ();

      blockLoop = blockLoopCount = 0;
      cellList = getThread(threadID);

      #if defined(DEBUG)
      std::cout << "\nCell Count (" << threadID << "):  " << cellList->size() << "\n";
      #endif

      for(deque< Cell * >::iterator cellIterator = cellList->begin(); cellIterator != cellList->end(); cellIterator++)
      {
         std::deque< Instruction * > *tempInstructionList = new std::deque< Instruction * >();
         std::deque< UINT_32 >  *conflictDistances = new std::deque< UINT_32 >();

         //Check to see if this is the start of a cell loop & insert loop-begin around multiple blocks -- For compression
         if((*cellIterator)->getCellType() == LoopStart)
         {
            std::cout << "Loop Start (" << (*cellIterator)->getLoopCount() << ")\n";

            blockLoop = (*cellIterator)->getLoopCount();
            instruction = new Instruction(iBeginLoop);
            instruction->set_subCode(BlockLoop);
            instruction->set_instructionID(blockLoopCount);
            tempInstructionList->push_back(instruction);

            blockLoopCount = blockLoopCount + 1;

            //add this instruction stream to the thread's instruction stream and remove it
            instructionList->insert(instructionList->end(), tempInstructionList->begin(), tempInstructionList->end());
            delete tempInstructionList;

            continue;
         }
         else if((*cellIterator)->getCellType() == LoopEnd)
         {
            std::cout << "Loop End\n";

            instruction = new Instruction(iEndLoop);
            instruction->set_subCode(BlockLoop);
            instruction->set_iterations(blockLoop);
            tempInstructionList->push_back(instruction);

            //add this instruction stream to the thread's instruction stream and remove it
            instructionList->insert(instructionList->end(), tempInstructionList->begin(), tempInstructionList->end());
            delete tempInstructionList;

            blockLoop = 0;

            continue;
         }

         //Check to see if there is anything in the cell
         if((*cellIterator)->getNumInstructions() < 1)
            continue;

         ///FIXME We would like this specified by the user as part of the input
         BOOL useLoops = 1;

         //Although they are reset below, these variables need to be 0 at the start of every loop -- MAYBE
         numMemOps = numFloatOps = numIntegerOps = numLoadOps = numStoreOps = 0;
         sharedReads = uniqueReads = sharedWrites = uniqueWrites = 0;

         #if defined(DEBUG)
         std::cout << "Thread ID:  " << threadID << "\t" << "Instructions:  " << (*cellIterator)->getNumInstructions() << "\t";

         std::cout << "INT:  " << (*cellIterator)->getNumIntegerOps() << "   ";
         std::cout << "FLT:  " << (*cellIterator)->getNumFloatingPointOps() << "   ";
         std::cout << "MEM:  " << (*cellIterator)->getNumMemoryOps() << "  L" << (*cellIterator)->getNumUniqueReads() << "  S" << (*cellIterator)->getNumUniqueWrites() << "   ";
         std::cout << "TX:  "  << (*cellIterator)->getCellType();

         #if not defined(VERBOSE)
         std::cout << std::endl;
         #else
         std::cout << std::flush;
         #endif
         #endif

         numMemOps = (*cellIterator)->getNumMemoryOps();
         numFloatOps = (*cellIterator)->getNumFloatingPointOps();
         numIntegerOps = (*cellIterator)->getNumIntegerOps();

         sharedReads = (*cellIterator)->getNumSharedReads();
         sharedWrites = (*cellIterator)->getNumSharedWrites();
         uniqueReads = (*cellIterator)->getNumUniqueReads();
         uniqueWrites = (*cellIterator)->getNumUniqueWrites();

         //the number of memory operations must be GREATER than the sum of MAX(loads) + MAX(stores)
         if(numMemOps <= (std::max(sharedReads, uniqueReads) + std::max(sharedWrites, uniqueWrites)))
         {
            numMemOps = numMemOps + 1;

            if(numIntegerOps > 0)
               numIntegerOps = numIntegerOps - 1;
            else if(numFloatOps > 0)
               numFloatOps = numFloatOps - 1;
            else
            {
               std::cerr << "\n\t\t               ---MEMORY FAILURE---" << "\n";
               std::cerr << "\t\t         ---Instruction Ratios Changed---" << "\n";
               std::cerr << "\t\t---Tx Stride And Tx Granularity May Be Affected---" << std::endl;
            }
         }

         //check for breakage
         I(numMemOps > (std::max(sharedReads, uniqueReads) + std::max(sharedWrites, uniqueWrites)));
         I(uniqueReads  >= sharedReads);
         I(uniqueWrites >= sharedWrites);

         /* Variables -- Loop Body Only */
         UINT_64  totalIns = numMemOps + numFloatOps + numIntegerOps;
         float    pctMem = float(numMemOps) / totalIns;
         float    pctFlt = float(numFloatOps) / totalIns;
         float    pctInt = float(numIntegerOps) / totalIns;

         float    minMem = uniqueReads + uniqueWrites;
         float    minTotal = minMem / pctMem;
         float    minFlt = pctFlt * minTotal;
         float    minInt = pctInt * minTotal;

         //loops require 3 integer ALU ops and 1 branch per cell
         if(minInt < EXTRA_LOOP_OPS && useLoops == 1)
         {
            minInt = 4.0;

            if(pctInt == 0)
               pctInt = float(minInt) / totalIns;

            minTotal = minInt / pctInt;
            minMem = pctMem * minTotal;
            minFlt = pctFlt * minTotal;

            minTotal = minTotal - 1.0;
         }

         INT_32 loopSize = roundFloat(minTotal);
         INT_32 numIters = totalIns / loopSize;
         INT_32 remainder = totalIns % loopSize;

         //If loops are enabled, there are additional checks that need to happen
         if(useLoops == 1 && totalIns >= (2 * loopSize))
            useLoops = 1;
         else
            useLoops = 0;

         //Check to make sure that the branch target is withing range
         if(useLoops == 1 && loopSize > _16_BIT_RANGE)
         {
            std::cerr << "\n\t\t               ---LOOP FAILURE---" << "\n";
            std::cerr << "\t\t   ---Loop Size Exceeded Addressable Range---" << std::endl;
            exit(0);
         }

         //disable loops if the memory outlay is specified -- looping managed independantly
         if((*cellIterator)->getConflictModel() == Specified)
            useLoops = 0;

         if(useLoops == 1)
         {
            numMemOps = roundFloat(minMem);
            numFloatOps = roundFloat(minFlt);
            numIntegerOps = roundFloat(minInt) - EXTRA_LOOP_OPS;
         }

         //ensure that the number of R/W operations is correct
         if(uniqueWrites > 0 && uniqueReads > 0 && sharedReads < uniqueReads && sharedWrites < uniqueWrites)
         {
            numLoadOps  = numMemOps * 5 / 7;
            numStoreOps = numMemOps - numLoadOps;
         }
         else if(uniqueReads > 0 && sharedReads == uniqueReads)
         {
            numLoadOps = uniqueReads;
            numStoreOps = numMemOps - numLoadOps;
         }
         else if(uniqueWrites > 0 && sharedWrites == uniqueWrites)
         {
            numStoreOps = uniqueWrites;
            numLoadOps = numMemOps - numStoreOps;
         }
         else if(uniqueWrites > 0)
         {
            numLoadOps  = 0;
            numStoreOps = numMemOps;
         }
         else if(uniqueReads > 0)
         {
            numStoreOps = 0;
            numLoadOps  = numMemOps;
         }
         else
         {
            std::cerr << "Ow! -- R(" << uniqueReads << ") W(" << uniqueWrites << ")" << "\n";
         }

         if(numLoadOps < std::max(sharedReads, uniqueReads))
         {
            numLoadOps = std::max(sharedReads, uniqueReads);
            numStoreOps = numMemOps - numLoadOps;
         }
         else if(numStoreOps < std::max(sharedWrites, uniqueWrites))
         {
            numStoreOps = std::max(sharedWrites, uniqueWrites);
            numLoadOps = numMemOps - numStoreOps;
         }

         //recalculate based on instruction adjustments
         loopSize = numMemOps + numFloatOps + numIntegerOps + EXTRA_LOOP_OPS;
         numIters = roundFloat(totalIns) / loopSize;
         remainder = roundFloat(totalIns) % loopSize;

         //Check to make sure that the branch target is withing range
         if(useLoops == 1 && loopSize > _16_BIT_RANGE)
         {
            std::cerr << "\n\t\t               ---LOOP FAILURE---" << "\n";
            std::cerr << "\t\t   ---Loop Size Exceeded Addressable Range---" << std::endl;
            exit(0);
         }

         //check for range (16 bit max immediate)
         if(numIters > _16_BIT_RANGE)
         {
            std::cerr << "\nNumber of loop iterations exceeded range (" << _16_BIT_RANGE << "). Recalculating parameters.\n";
            std::cerr << "NOTE: This is experimental and may produce cells too large to compile. It may also modify stride." << std::endl;

            std::cerr << "LOOP SIZE:  " << loopSize;
            std::cerr << "\tITERATIONS:  " << numIters;
            std::cerr << "\tREMAINDER  : " << remainder;
            std::cerr << "\n" << std::endl;

            //recalculate based on loop adjustments
            minTotal = (totalIns / float(_16_BIT_RANGE)) + 2.0;
            minMem   = pctMem * minTotal;
            minFlt   = pctFlt * minTotal;
            minInt   = pctInt * minTotal;

            numMemOps      = roundFloat(minMem);
            numFloatOps    = roundFloat(minFlt);
            numIntegerOps  = roundFloat(minInt) - EXTRA_LOOP_OPS;

            //if the number of operations has changed then the number of R/W operations must be recalculated
            if(uniqueWrites > 0 && uniqueReads > 0)
            {
               numLoadOps = numMemOps * 5 / 7;
               numStoreOps = numMemOps - numLoadOps;
            }
            else if(uniqueWrites > 0)
            {
               numLoadOps = 0;
               numStoreOps = numMemOps;
            }
            else if(uniqueReads > 0)
            {
               numStoreOps = 0;
               numLoadOps = numMemOps;
            }

            if(numLoadOps < std::max(sharedReads, uniqueReads))
            {
               numLoadOps = std::max(sharedReads, uniqueReads);
               numStoreOps = numMemOps - numLoadOps;
            }
            else if(numStoreOps < std::max(sharedWrites, uniqueWrites))
            {
               numStoreOps = std::max(sharedWrites, uniqueWrites);
               numLoadOps = numMemOps - numStoreOps;
            }

            loopSize = numMemOps + numFloatOps + numIntegerOps + EXTRA_LOOP_OPS;
            numIters = totalIns / loopSize;
            remainder = totalIns % loopSize;

            //Check to make sure that the branch target is withing range
            if(useLoops == 1 && loopSize > _16_BIT_RANGE)
            {
               std::cerr << "\n\t\t               ---LOOP FAILURE---" << "\n";
               std::cerr << "\t\t   ---Loop Size Exceeded Addressable Range---" << std::endl;
               exit(0);
            }
         }
         //END Range check

         //find remainder local minimum
         //this algorithm attempts to minimize the number of remainder instructions -- reducing program size
         if(useLoops == 1 && numIters > 1 && float(remainder) / float(loopSize) >= .65)
         {
            int newS = 0;
            int oldS = 0;
            int newR = 0;
            int oldR = 0;
            int newI = 0;
            int oldI = 0;
            unsigned int testMe = 0;

            #if defined(VERBOSE)
            std::cout << "\n****Remainder:  " << float(remainder) << "\tSize:  " << float(loopSize) << "(" << minTotal << ")";
            std::cout << "\tIters:  " << numIters << "\tPercent:  " << float(remainder) / float(loopSize) << std::endl;

            std::cout << "***Old InsCount:  " << totalIns << "\tNew InsCount:  " << (numIters + 1) * loopSize;
            std::cout << "\tNew LoopSz:  " << totalIns/(numIters + 1) << "\tNew Iters:  " << numIters + 1 << std::endl;

            std::cout << "***MEM:  " << pctMem << "\tINT:  " << pctInt << "\tFLT:  " << pctFlt << std::endl;
            #endif

            do
            {
               if(testMe == 0)
               {
                  oldS = loopSize;
                  oldI = numIters;
                  oldR = remainder;
                  testMe = 1;
               }
               else
               {
                  oldS = newS;
                  oldI = newI;
                  oldR = newR;
               }

               newS = oldS + 10;
               newI = roundFloat(totalIns) / newS;
               newR = roundFloat(totalIns) % newS;

               #if defined(VERBOSE)
               std::cout << "\tS -- " << oldS << "   " << newS << "\n";
               std::cout << "\tI -- " << oldI << "   " << newI << "\n";
               std::cout << "\tR -- " << oldR << "   " << newR << "\n";
               #endif

            }while(newR < oldR);

            #if defined(VERBOSE)
            std::cout << "+++NewRemainder(" << remainder << "):  " << oldR;
            std::cout << "\tnewSize(" << loopSize << "):  " << oldS;
            std::cout << "\tnewIters(" << numIters << "):  " << oldI << std::endl;
            #endif

            if(oldS >= minTotal + EXTRA_LOOP_OPS)
            {
               numMemOps = roundFloat(oldS * pctMem);
               numFloatOps = roundFloat(oldS * pctFlt);
               numIntegerOps = roundFloat(oldS * pctInt) - EXTRA_LOOP_OPS;

               if(uniqueWrites > 0 && uniqueReads > 0)
               {
                  numLoadOps = numMemOps * 5 / 7;
                  numStoreOps = numMemOps - numLoadOps;
               }
               else if(uniqueWrites > 0)
               {
                  numLoadOps = 0;
                  numStoreOps = numMemOps;
               }
               else if(uniqueReads > 0)
               {
                  numStoreOps = 0;
                  numLoadOps = numMemOps;
               }
               else
               {
                  std::cerr << "Ow! -- R(" << uniqueReads << ") W(" << uniqueWrites << ")" << "\n";
               }

               if(numLoadOps < std::max(sharedReads, uniqueReads))
               {
                  numLoadOps = std::max(sharedReads, uniqueReads);
                  numStoreOps = numMemOps - numLoadOps;
               }
               else if(numStoreOps < std::max(sharedWrites, uniqueWrites))
               {
                  numStoreOps = std::max(sharedWrites, uniqueWrites);
                  numLoadOps = numMemOps - numStoreOps;
               }

               //recalculate based on instruction adjustments
               loopSize = numMemOps + numFloatOps + numIntegerOps + EXTRA_LOOP_OPS;
               numIters = roundFloat(totalIns) / loopSize;
               remainder = roundFloat(totalIns) % loopSize;

               #if defined(VERBOSE)
               std::cout << "\n@@@@Remainder:  " << float(remainder) << "\tSize:  " << float(loopSize) << "(" << minTotal << ")";
               std::cout << "\tIters:  " << numIters << "\tPercent:  " << float(remainder) / float(loopSize) << std::endl;

               std::cout << "@@@@Old Size:  " << roundFloat(totalIns) << "\tNew Size:  " << roundFloat(float(remainder) + float(loopSize) * numIters);
               std::cout << "\tDiff:  " << roundFloat(totalIns) - roundFloat(float(remainder) + float(loopSize) * numIters) << std::endl;
               #endif
            }
         }
         //END Finding Local Minimum

         #if defined(DEBUG)
         std::cout << "   LD-" << numLoadOps << "(S" << sharedReads << " - U" << uniqueReads << ")";
         std::cout << " ST-" << numStoreOps << "(S" << sharedWrites << " - U" << uniqueWrites << ")";
         if((*cellIterator)->getConflictModel() == High)
            std::cout << " -H- ";
         else if((*cellIterator)->getConflictModel() == Random)
            std::cout << " -R- ";
         else
            std::cout << " -S- ";
         std::cout << "\n";
         #endif

         #if defined(VERBOSE)
         std::cout << "LOOP SIZE:  " << loopSize;
         std::cout << "\tITERATIONS:  " << numIters;
         std::cout << "\tREMAINDER  : " << remainder;
         std::cout << "\n" << std::endl;
         #endif

         //insert special Loop-Init instruction
         if(useLoops == 1)
         {
            instruction = new Instruction(iInitLoop);
            instruction->set_subCode(CellLoop);
            tempInstructionList->push_back(instruction);
         }

         //insert special TX-Begin instruction -- this should come after the loop init
         if((*cellIterator)->getCellType() == Transactional)
         {
            instruction = new Instruction(iBeginTX);
            if((*cellIterator)->getConflictModel() == High)
               instruction->set_conflictModel(High);
            tempInstructionList->push_back(instruction);
         }

         //insert special Loop-Begin instruction
         if(useLoops == 1)
         {
            instruction = new Instruction(iBeginLoop);
            instruction->set_subCode(CellLoop);
            tempInstructionList->push_back(instruction);
         }

         //generate load instructions
         for(UINT_32 insCount = 0; insCount < numLoadOps; insCount++)
         {
            instruction = new Instruction(iLoad);
            tempInstructionList->push_back(instruction);
         }

         //generate integer instructions
         for(UINT_32 insCount = 0; insCount < numIntegerOps; insCount++)
         {
            instruction = new Instruction(iALU);
            tempInstructionList->push_back(instruction);
         }

         //generate floating-point instructions
         for(UINT_32 insCount = 0; insCount < numFloatOps; insCount++)
         {
            instruction = new Instruction(fpALU);
            tempInstructionList->push_back(instruction);
         }

         //generate store instructions
         for(UINT_32 insCount = 0; insCount < numStoreOps; insCount++)
         {
            instruction = new Instruction(iStore);
            tempInstructionList->push_back(instruction);
         }

         //insert special Loop-End instruction
         if(useLoops == 1)
         {
            instruction = new Instruction(iEndLoop);
            instruction->set_subCode(CellLoop);
            instruction->set_iterations(numIters);
            tempInstructionList->push_back(instruction);

            if(remainder > 0)
            {
               UINT_32 remMemOps_ = roundFloat(remainder * pctMem);
               UINT_32 remIntOps_ = roundFloat(remainder * pctInt);
               UINT_32 remFltOps_ = roundFloat(remainder * pctFlt);

               while(remMemOps_ + remIntOps_ + remFltOps_ > remainder)
               {
                  remIntOps_ = remIntOps_ - 1;
               }

               while(remMemOps_ + remIntOps_ + remFltOps_ < remainder)
               {
                  remIntOps_ = remIntOps_ + 1;
               }

               //generate integer instructions
               for(UINT_32 insCount = 0; insCount < remIntOps_; insCount++)
               {
                  instruction = new Instruction(iALU);
                  tempInstructionList->push_back(instruction);
               }

               //generate floating-point instructions
               for(UINT_32 insCount = 0; insCount < remFltOps_; insCount++)
               {
                  instruction = new Instruction(fpALU);
                  tempInstructionList->push_back(instruction);
               }

               //generate memory instructions
               if(uniqueReads > 0)
               {
                  for(UINT_32 insCount = 0; insCount < remMemOps_; insCount++)
                  {
                     instruction = new Instruction(iLoad);
                     tempInstructionList->push_back(instruction);
                  }
               }
               else if(uniqueWrites > 0)
               {
                  for(UINT_32 insCount = 0; insCount < remMemOps_; insCount++)
                  {
                     instruction = new Instruction(iStore);
                     tempInstructionList->push_back(instruction);
                  }
               }
               else
               {
                  for(UINT_32 insCount = 0; insCount < remMemOps_; insCount++)
                  {
                     instruction = new Instruction(iALU);
                     tempInstructionList->push_back(instruction);
                  }
               }
            }
         }

         //insert special TX-End instruction
         if((*cellIterator)->getCellType() == Transactional)
         {
            instruction = new Instruction(iCommitTX);
            tempInstructionList->push_back(instruction);
         }

         ///The following operations CANNOT be reordered because of boundary cases figure out
         ///which locations are shared and private. In addition, no shared references
         ///should ever occur outside of the loop boundary unless the conflict model is high.
         privatizeMemory(tempInstructionList, sharedReads, sharedWrites);

         //figure out which locations are unique
         localizeMemory(tempInstructionList, uniqueReads, uniqueWrites);

         //If the memory layout is specified, we need to do extra work
         if((*cellIterator)->getConflictModel() == Specified)
         {
            //randomize contents
            if(randomizeStream == 1)
               randomizeInstructionStream(tempInstructionList, (*cellIterator)->getConflictModel(), (*cellIterator)->getCellType(), sharedReads, sharedWrites, useLoops);

            //if the layout is specified, loads and stores should be reordered
            specifyMemory(tempInstructionList, (*cellIterator)->get_loadConflictList(), (*cellIterator)->get_storeConflictList());

            //need to be sure that no load occurs before the first unique load and that no store occurs before the first unique store
            prioritizeMemory(tempInstructionList, (*cellIterator)->get_loadConflictList(), (*cellIterator)->get_storeConflictList());

//             getDistanceList(tempInstructionList, (*cellIterator)->get_loadConflictList(), (*cellIterator)->get_storeConflictList(), conflictDistances);
//             makeStrideLoops(tempInstructionList, conflictDistances);
         }
         else
         {
            //setup confict region
            if((*cellIterator)->getConflictModel() == High)
            {
               conflictizeMemory(tempInstructionList, uniqueReads, uniqueWrites, useLoops, numIters, remainder, loopSize);
            }

            //randomize contents
            if(randomizeStream == 1)
               randomizeInstructionStream(tempInstructionList, (*cellIterator)->getConflictModel(), (*cellIterator)->getCellType(), sharedReads, sharedWrites, useLoops);
         }

         //add a the branch instruction
         instruction = new Instruction(iBJ);
         tempInstructionList->push_back(instruction);

         //add this instruction stream to the thread's instruction stream and remove it
         instructionList->insert(instructionList->end(), tempInstructionList->begin(), tempInstructionList->end());
         delete tempInstructionList;
      }
      //END Converting cells to instructions

      //add the stream to the per-thread list of instructions
      perThread_instructionList.push_back(instructionList);

      #if defined(VERBOSE)
      std::cout << "List Size:  " << instructionList->size() << "\n";
      #endif
   }

   std::cout << "COMPLETE" << std::endl;
}
//END spinalColumn


/**
 * @ingroup Skin
 * 
 * @param instructionList 
 * @param conflictDistances 
 */
void Skin::makeStrideLoops(std::deque < Instruction * > *instructionList, std::deque< UINT_32 > *conflictDistances)
{
   /* Variables */
   BOOL  useLoops = 0;
   UINT_64  shiftAmount = 0;
   UINT_64  streamStart, streamEnd;
   const UINT_32 MIN_LOOP_SIZE = 20;

   /* Processes */
   for(UINT_32 counter_a = 0, counter_b = 1; counter_b < conflictDistances->size(); counter_a++, counter_b++)
   {
      //Figure out the streamStart and streamEnd points
      streamStart = conflictDistances->at(counter_a);
      streamEnd   = conflictDistances->at(counter_b);

      //check to see if the distance between two successive uniques is greater than the minimum size required for a loop
      if(streamEnd - streamStart > MIN_LOOP_SIZE)
      {
         UINT_64  listStart = streamStart - shiftAmount;
         UINT_64  listEnd   = streamEnd - shiftAmount;

         /* Variables -- Loop Body Only */
         UINT_64  totalIns = streamEnd - streamStart - 1 - 1;
         float    pctMem;
         float    pctFlt;
         float    pctInt;

         float    minMem;
         float    minTotal;
         float    minFlt;
         float    minInt;

         UINT_32  mem_ops = 0;
         UINT_32  ld_ops = 0;
         UINT_32  st_ops = 0;
         UINT_32  alu_ops = 0;
         UINT_32  fp_ops = 0;
         UINT_32  sharedReads = 0;
         UINT_32  sharedWrites = 0;

std::cout << "\n*****LOOP of size " << totalIns;
UINT_32 MY_LIST_SIZE = instructionList->size();
std::cout << "\n------------LIST SIZE:  " << MY_LIST_SIZE << "\n\n" << std::endl;
         //find the ins. mix for this interval
         for(UINT_32 instructionList_counter = listStart + 1; instructionList_counter < listEnd; instructionList_counter++)
         {
// std::cout << "COUNTER:  " << instructionList_counter << "\n";
            switch(instructionList->at(instructionList_counter)->get_opCode())
            {
               case iLoad :
                  mem_ops = mem_ops + 1;
                  if(instructionList->at(instructionList_counter)->get_isShared() == 1)
                     sharedReads = sharedReads + 1;
                  break;
               case iStore :
                  mem_ops = mem_ops + 1;
                  if(instructionList->at(instructionList_counter)->get_isShared() == 1)
                     sharedWrites = sharedWrites + 1;
                  break;
               case iALU :
                  alu_ops = alu_ops + 1;
                  break;
               case fpALU :
                  fp_ops = fp_ops + 1;
                  break;
            }
         }
std::cout << "\nShared Reads:  " << sharedReads << "\tsharedWrites:  " << sharedWrites << "\n";
std::cout << "MEM:  " << mem_ops << "    ALUS:  " << alu_ops << "    FPS:  " << fp_ops << "    total:  " << totalIns  << std::endl;

         //set the general instruction mix
         if(mem_ops + fp_ops + alu_ops > totalIns)
         {
            UINT_32 tempTotal = mem_ops + fp_ops + alu_ops;

            pctMem = float(mem_ops) / tempTotal;
            pctFlt = float(fp_ops) / tempTotal;
            pctInt = float(alu_ops) / tempTotal;
         }
         else
         {
            pctMem = float(mem_ops) / totalIns;
            pctFlt = float(fp_ops) / totalIns;
            pctInt = float(alu_ops) / totalIns;
         }

         //calculate minimum ins. counts
         minMem = sharedReads + sharedWrites;
         if(minMem > 0)
         {
            minTotal = minMem / pctMem;
            minFlt = pctFlt * minTotal;
            minInt = pctInt * minTotal;
         }
         else
         {
            minTotal = 4.0;
            minFlt = 0.0;
            minInt = 4.0;
         }


std::cout << "pctMem: " << pctMem << "    pctFlt:  " << pctFlt << "    pctInt:  " << pctInt;
std::cout << "   <>   " << "minTotal:  " << minTotal << "    minMem:  " << minMem << "    minFlt:  " << minFlt << "    minInt:  " << minInt << "\n";

         //loops require 3 integer ALU ops and 1 branch per cell
         if(minInt < EXTRA_LOOP_OPS)
         {
            minInt = 4.0;

            if(pctInt == 0)
               pctInt = float(minInt) / totalIns;

            minTotal = minInt / pctInt;
            minMem = pctMem * minTotal;
            minFlt = pctFlt * minTotal;

            minTotal = minTotal - 1.0;
         }

         INT_32 loopSize = roundFloat(minTotal);
         INT_32 numIters = totalIns / loopSize;
         INT_32 remainder = totalIns % loopSize;

std::cout << "\tLoop Size:  " << loopSize << "    Iterations:  " << numIters << "    Remainder:  " << remainder << "    total:  " << totalIns << std::endl;

         //If loops are enabled, there are additional checks that need to happen
         if(totalIns >= (2 * loopSize))
            useLoops = 1;
         else
            useLoops = 0;

         if(useLoops == 1)
         {
            if(minMem > 1.0)
            {
               ld_ops = roundFloat(minMem * 5 / 7);
               st_ops = roundFloat(minMem - ld_ops);
            }
            else if(pctMem > 0.0)
            {
               ld_ops = 1;
            }

            if(minFlt > 1.0)
            {
               fp_ops = roundFloat(minFlt);
            }
            else if(pctFlt > 0.0)
            {
               fp_ops = 1;
            }

            alu_ops = roundFloat(minInt) - EXTRA_LOOP_OPS;

         //recalculate loop size after ensuring instruction ratios
         loopSize = ld_ops + st_ops + fp_ops + alu_ops + EXTRA_LOOP_OPS;
         numIters = totalIns / loopSize;
         remainder = totalIns % loopSize;

std::cout << "\tld_ops:  " << ld_ops << "    st_ops:  " << st_ops << "    fp_ops:  " << fp_ops << "    alu_ops:  " << alu_ops << "\n";
std::cout << "\tLoop Size:  " << loopSize << "    Iterations:  " << numIters << "    Remainder:  " << remainder << "\n";
         }

         //now the intervening instructions need to be excised and replaced with the new loop operations and normal instructions
         //NOTE the beginning must include the memory operation as well as the streamStart of the transaction
         //NOTE the streamEnd is inclusive
         if(useLoops == 1)
         {
            shiftAmount = shiftAmount + (totalIns + 1 - (ld_ops + st_ops + fp_ops + alu_ops + 3) - remainder);

            Instruction *instruction;
            std::deque< Instruction * > *temp_instructionList = new std::deque< Instruction * > ();

            instructionList->erase(instructionList->begin() + listStart + 1, instructionList->begin() + listEnd);

            //generate load instructions
            for(UINT_32 insCount = 0; insCount < ld_ops; insCount++)
            {
               instruction = new Instruction(iLoad);
               temp_instructionList->push_back(instruction);
            }

            //generate integer instructions
            for(UINT_32 insCount = 0; insCount < alu_ops; insCount++)
            {
               instruction = new Instruction(iALU);
               temp_instructionList->push_back(instruction);
            }

            //generate floating-point instructions
            for(UINT_32 insCount = 0; insCount < fp_ops; insCount++)
            {
               instruction = new Instruction(fpALU);
               temp_instructionList->push_back(instruction);
            }

            //generate store instructions
            for(UINT_32 insCount = 0; insCount < st_ops; insCount++)
            {
               instruction = new Instruction(iStore);
               temp_instructionList->push_back(instruction);
            }

            randomizeInstructionStream(temp_instructionList, Random, Sequential, 0, 0, 0);

            //Looping
            UINT_32 randomID = rand() % 1000 +  rand() % 100;                 //need to add a random loop ID
            instruction = new Instruction(iInitLoop);                         //added OUTSIDE of the loop body -- needs to be factored into the Tx ins. count
            instruction->set_subCode(CellLoop);
            instructionList->insert(instructionList->begin() + listStart + 1, instruction);
            instruction = new Instruction(iBeginLoop);
            instruction->set_subCode(MultiCellLoop);
            instruction->set_instructionID(randomID);
            instructionList->insert(instructionList->begin() + listStart + 2, instruction);

            for(std::deque< Instruction * >::iterator meeces = temp_instructionList->begin(); meeces != temp_instructionList->end(); meeces++)
            {
               instructionList->insert(instructionList->begin() + listStart + 3, *meeces);
            }

            instruction = new Instruction(iEndLoop);
            instruction->set_subCode(MultiCellLoop);
            instruction->set_iterations(numIters);
            instructionList->insert(instructionList->begin() + listStart + 3 + temp_instructionList->size(), instruction);
std::cout << "----------------------------REMAINDER:  " << remainder << std::endl;
            if(remainder > 0)
            {
               delete temp_instructionList;
               temp_instructionList = new std::deque< Instruction * > ();

               UINT_32 remMemOps_ = roundFloat(remainder * pctMem);
               UINT_32 remIntOps_ = roundFloat(remainder * pctInt);
               UINT_32 remFltOps_ = roundFloat(remainder * pctFlt);

               while(remMemOps_ + remIntOps_ + remFltOps_ > remainder)
               {
                  remIntOps_ = remIntOps_ - 1;
               }

               while(remMemOps_ + remIntOps_ + remFltOps_ < remainder)
               {
                  remIntOps_ = remIntOps_ + 1;
               }

               //generate integer instructions
               for(UINT_32 insCount = 0; insCount < remIntOps_; insCount++)
               {
                  instruction = new Instruction(iALU);
                  temp_instructionList->push_back(instruction);
               }

               //generate floating-point instructions
               for(UINT_32 insCount = 0; insCount < remFltOps_; insCount++)
               {
                  instruction = new Instruction(fpALU);
                  temp_instructionList->push_back(instruction);
               }

               //generate memory instructions
               for(UINT_32 insCount = 0; insCount < remMemOps_; insCount++)
               {
                  instruction = new Instruction(iALU);
                  temp_instructionList->push_back(instruction);
               }

               //insert after the loop and after the loop streamEnd instruction
               for(std::deque< Instruction * >::iterator meeces = temp_instructionList->begin(); meeces != temp_instructionList->end(); meeces++)
               {
                  instructionList->insert(instructionList->begin() + listStart + loopSize, *meeces);
               }
std::cout << "\n" << std::endl;
            }//END remainder

            delete temp_instructionList;
         }//END loops
      }//END interval
   }

   //cleanup on aisle 38
   delete conflictDistances;
}
//END


/**
 * @ingroup Skin
 * 
 * @note  Generates a sorted list containing the sizes of the areas between unique memory references
 * @param instructionList 
 * @param readConflictList 
 * @param writeConflictList 
 * @param conflictDistances 
 */
void Skin::getDistanceList(std::deque < Instruction * > *instructionList, std::list< CONFLICT_PAIR >* readConflictList, std::list< CONFLICT_PAIR >* writeConflictList, std::deque<UINT_32> *conflictDistances)
{
   /* Variables */

   /* Processes */
   for(std::list< CONFLICT_PAIR >::const_iterator conflictList_it = readConflictList->begin(); conflictList_it != readConflictList->end(); conflictList_it++)
   {
      conflictDistances->push_back(conflictList_it->second);
   }

   for(std::list< CONFLICT_PAIR >::const_iterator conflictList_it = writeConflictList->begin(); conflictList_it != writeConflictList->end(); conflictList_it++)
   {
      conflictDistances->push_back(conflictList_it->second);
   }

   std::sort(conflictDistances->begin(), conflictDistances->end());

   #if defined(VERBOSE)
   std::cout << "\n\tDELTAS\n";
   for(std::deque<UINT_32>::iterator booWhoHoo = conflictDistances->begin(); booWhoHoo != conflictDistances->end(); booWhoHoo++)
   {
      std::cout << "\t   " << *booWhoHoo << "\n";
   }
   #endif
}
//END


/**
 * @ingroup Skin
 * 
 * @param  
 */
void Skin::insertVertebrae(void)
{
   /* Variables */
   UINT_32 count;
   std::deque< Instruction * >::iterator instructionList_it;

   /* Processes */
   static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (getRDTSC()));
//    static boost::lagged_fibonacci1279 generator(static_cast<unsigned> (42));
   boost::uniform_int<> FP_Distribution(FP2, FP20);
   boost::uniform_int<> ALU_Distribution(T2, S5);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_int<> >  FP_Register(generator, FP_Distribution);
   boost::variate_generator<boost::lagged_fibonacci1279&, boost::uniform_int<> >  ALU_Register(generator, ALU_Distribution);

   std::cout << "Inserting vertebrae for thread ";
   for(THREAD_ID threadID = 0; threadID < numThreads; threadID++)
   {
      std::cout << threadID << "..." << std::flush;

      count = 0;
      std::deque < Instruction * > *instructionList = perThread_instructionList[threadID];

//       std::cout << "\nThread:  " << threadID << endl;

      for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
      {
         if((*instructionList_it)->get_opCode() == iALU)
         {
            count = count + 1;
//             std::cout << "ALU " << count << "\n";

            (*instructionList_it)->set_rs(Instruction::identifyRegister(ALU_Register()));
            (*instructionList_it)->set_rt(Instruction::identifyRegister(ALU_Register()));
            (*instructionList_it)->set_rd(Instruction::identifyRegister(ALU_Register()));

//             std::cout << "RS:  " << (*instructionList_it)->get_rs() << "    ";
//             std::cout << "RT:  " << (*instructionList_it)->get_rt() << "    ";
//             std::cout << "RD:  " << (*instructionList_it)->get_rd() << "\n";
         }
         else if((*instructionList_it)->get_opCode() == fpALU)
         {
            count = count + 1;
//             std::cout << "FP " << count << "\n";

            (*instructionList_it)->set_rs(Instruction::identifyRegister(FP_Register()));
            (*instructionList_it)->set_rt(Instruction::identifyRegister(FP_Register()));
            (*instructionList_it)->set_rd(Instruction::identifyRegister(FP_Register()));

//             std::cout << "RS:  " << (*instructionList_it)->get_rs() << "    ";
//             std::cout << "RT:  " << (*instructionList_it)->get_rt() << "    ";
//             std::cout << "RD:  " << (*instructionList_it)->get_rd() << "\n";
         }
         else if((*instructionList_it)->get_opCode() == iLoad)
         {
            count = count + 1;
//             std::cout << "LD " << count << " (" << (*instructionList_it)->get_isShared() << " - " << (*instructionList_it)->get_isUnique() << ")\n";

            (*instructionList_it)->set_rd(Instruction::identifyRegister(FP_Register()));

//             std::cout << "RS:  " << (*instructionList_it)->get_rs() << "    ";
//             std::cout << "RT:  " << (*instructionList_it)->get_rt() << "    ";
//             std::cout << "RD:  " << (*instructionList_it)->get_rd() << "\n";
         }
         else if((*instructionList_it)->get_opCode() == iStore)
         {
            count = count + 1;
//             std::cout << "ST " << count << " (" << (*instructionList_it)->get_isShared() << " - " << (*instructionList_it)->get_isUnique() << ")\n";

            (*instructionList_it)->set_rd(Instruction::identifyRegister(FP_Register()));

//             std::cout << "RS:  " << (*instructionList_it)->get_rs() << "    ";
//             std::cout << "RT:  " << (*instructionList_it)->get_rt() << "    ";
//             std::cout << "RD:  " << (*instructionList_it)->get_rd() << "\n";
         }

      }
   }

   std::cout << "COMPLETE" << std::endl;
}
//END insertVertebrae


/**
 * @ingroup Skin
 * 
 * @note  Only guarauntee for closest placement
 * @param  
 */
void Skin::synchronize(void)
{
   /* Variables */
   UINT_32 numInstructions;
   UINT_32 barrierCount;
   UINT_32 barrierStride;
   UINT_32 count_a, count_b;

   /* Processes */
   std::cout << "Synchronizing thread ";
   for(THREAD_ID threadID = 0; threadID < numThreads; threadID++)
   {
      std::cout << threadID << "..." << std::flush;

      std::deque < Instruction * > *instructionList = perThread_instructionList[threadID];

      numInstructions = instructionList->size();
      barrierCount = config->read<int> ( "Global" , "numBarriers" );
      barrierStride = numInstructions / barrierCount;

      #if defined(VERBOSE)
      std::cerr << barrierCount << "  <>  " << barrierStride << std::endl;
      std::cerr << "SIZE:  " << numInstructions << std::endl;
      #endif

      #if defined(VERBOSE)
      UINT_32 count_s = 0;
      #endif

      count_a = 0;
      count_b = barrierStride;
      for(std::deque < Instruction * >::iterator instList_it = instructionList->begin(); instList_it != instructionList->end(); instList_it++)
      {
         if(count_a < count_b)
         {
            count_a = count_a + 1;
            continue;
         }
         else
         {

            if(barrierCount > 1)
            {
               #if defined(VERBOSE)
               std::cerr << "barrCount:  " << barrierCount << "     count_a:  " << count_a << "\tcount_b:  " << count_b << "\n";
               #endif
               barrierCount = barrierCount - 1;
               count_b = count_a + barrierStride;
            }
            else
            {
               break;
            }

            #if defined(VERBOSE)
            count_s = count_a;
            #endif

            for(std::deque < Instruction * >::iterator instList_it_2 = instList_it; instList_it_2 != instructionList->end(); instList_it_2++)
            {
               if((*instList_it_2)->get_opCode() == iInitLoop)
               {
                  Instruction *tempInstruction = new Instruction(syncBarrier);
                  #if defined(VERBOSE)
                  std::cerr << "\tLoop:  " << barrierCount << "     count_a:  " << count_a << "\tcount_b:  " << count_b << "\tcount_s:  " << count_s << "\n";
                  #endif
                  instList_it = instList_it_2;
                  instructionList->insert(instList_it_2, tempInstruction);
                  break;
               }
               else if((*instList_it_2)->get_opCode() == iBeginTX)
               {
                  Instruction *tempInstruction = new Instruction(syncBarrier);
                  #if defined(VERBOSE)
                  std::cerr << "\tBegin:  " << barrierCount << "     count_a:  " << count_a << "\tcount_b:  " << count_b << "\tcount_s:  " << count_s << "\n";
                  #endif
                  instList_it = instList_it_2;
                  instructionList->insert(instList_it_2 - 2, tempInstruction);
                  break;
               }
               else if((*instList_it_2)->get_opCode() == iCommitTX)
               {
                  Instruction *tempInstruction = new Instruction(syncBarrier);
                  #if defined(VERBOSE)
                  std::cerr << "\tCommit:  " << barrierCount << "     count_a:  " << count_a << "\tcount_b:  " << count_b << "\tcount_s:  " << count_s << "\n";
                  #endif
                  instList_it = instList_it_2;
                  instructionList->insert(instList_it_2 + 2, tempInstruction);
                  break;
               }

               #if defined(VERBOSE)
               count_s = count_s + 1;
               #endif
            }
         }
      }
   }

   std::cout << "COMPLETE" << std::endl;
}

/**
 * @ingroup Skin
 * 
 * @param instructionList 
 * @param sharedLoads 
 * @param sharedStores 
 */
void Skin::privatizeMemory(std::deque < Instruction * > *instructionList, UINT_32 sharedLoads, UINT_32 sharedStores)
{
   /* Variables */
   std::deque< Instruction * >::iterator instructionList_it;

   /* Processes */
   for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
   {
      if((*instructionList_it)->get_opCode() == iLoad && sharedLoads > 0)
      {
         sharedLoads = sharedLoads - 1;
         (*instructionList_it)->set_isShared(1);
      }
      else if((*instructionList_it)->get_opCode() == iStore && sharedStores > 0)
      {
         sharedStores = sharedStores - 1;
         (*instructionList_it)->set_isShared(1);
      }
   }
}
//END privatizeMemory


/**
 * @ingroup Skin
 *
 * @param instructionList 
 * @param uniqueLoads 
 * @param uniqueStores 
 */
void Skin::localizeMemory(std::deque < Instruction * > *instructionList, UINT_32 uniqueLoads, UINT_32 uniqueStores)
{
   /* Variables */
   BOOL sharedLoad = 0;
   BOOL privLoad = 0;

   BOOL sharedStore = 0;
   BOOL privStore = 0;

   std::deque< Instruction * >::iterator instructionList_it;

   /* Processes */
   for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
   {
      if((*instructionList_it)->get_opCode() == iLoad && uniqueLoads > 0)
      {
         //if this instruction is shared and is the first one, then it is unique by default
         if((*instructionList_it)->get_isShared() == 1 && sharedLoad == 0)
         {
            sharedLoad = 1;
            uniqueLoads = uniqueLoads - 1;
            (*instructionList_it)->set_isUnique(1);
            continue;
         }
         else if((*instructionList_it)->get_isShared() == 1)
         {
            (*instructionList_it)->set_isUnique(1);
            uniqueLoads = uniqueLoads - 1;
            continue;
         }

         //if this instruction is private and is the first one, then it is unique by default
         if((*instructionList_it)->get_isShared() == 0 && privLoad == 0)
         {
            privLoad = 1;
            uniqueLoads = uniqueLoads - 1;
            (*instructionList_it)->set_isUnique(1);
            continue;
         }
         else if(privLoad == 1)
         {
            (*instructionList_it)->set_isUnique(1);
            uniqueLoads = uniqueLoads - 1;
         }
      }
      else if((*instructionList_it)->get_opCode() == iStore && uniqueStores > 0)
      {
         //if this instruction is shared and is the first one, then it is unique by default
         if((*instructionList_it)->get_isShared() == 1 && sharedStore == 0)
         {
            sharedStore = 1;
            uniqueStores = uniqueStores - 1;
            (*instructionList_it)->set_isUnique(1);
            continue;
         }
         else if((*instructionList_it)->get_isShared() == 1)
         {
            (*instructionList_it)->set_isUnique(1);
            uniqueStores = uniqueStores - 1;
            continue;
         }

         //if this instruction is private and is the first one, then it is unique by default
         if((*instructionList_it)->get_isShared() == 0 && privStore == 0)
         {
            privStore = 1;
            uniqueStores = uniqueStores - 1;
            (*instructionList_it)->set_isUnique(1);
            continue;
         }
         else if(privStore == 1)
         {
            (*instructionList_it)->set_isUnique(1);
            uniqueStores = uniqueStores - 1;
         }
      }
   }
}
//END localizeMemory


/**
 * @ingroup Skin
 * 
 * @param instructionList 
 * @param readConflictList 
 * @param writeConflictList 
 */
void Skin::specifyMemory(std::deque < Instruction * > *instructionList, std::list< CONFLICT_PAIR >* readConflictList, std::list< CONFLICT_PAIR >* writeConflictList)
{
   /* Variables */
   BOOL        isShared;
   ADDRESS_INT address;
   UINT_32     lineNumber;

   Instruction *tempInstruction;

   std::list< CONFLICT_PAIR >::const_iterator conflictList_it;
   std::deque< Instruction * >::iterator instructionList_it;

   /* Processes */
   //iterate through the list of conflicting LOADS and mark the instructions in the synthetic instruction list
   for(conflictList_it = readConflictList->begin(); conflictList_it != readConflictList->end(); conflictList_it++)
   {
      address = conflictList_it->first;
      lineNumber = conflictList_it->second;
      isShared = conflictList_it->first & 1;                      //bitwise AND operation shows whether the address is shared or not

      #if defined(DEBUG)
      std::cerr << "L(" << (conflictList_it->first & 1) << ")-Address:  " << std::hex << address << "  Line:  " << std::dec << lineNumber << std::endl;
      #endif

      for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
      {
         //make sure the instruction is a LOAD and has not been claimed
         if((*instructionList_it)->get_opCode() == iLoad && (*instructionList_it)->get_physicalAddress() == 0)
         {
            if(isShared == 0  && (*instructionList_it)->get_isShared() == 0 && (*instructionList_it)->get_isUnique() == 1)
            {
               //update the address for later use
               (*instructionList_it)->set_physicalAddress(address);

               //swap the instructions and exit the loop -- NOTE assume that BEGIN is the first instruciton, otherwise lineNumber will be different
               tempInstruction = instructionList->at(lineNumber);
               instructionList->at(lineNumber) = *instructionList_it;
               *instructionList_it = tempInstruction;
               break;
            }
            else if(isShared == 1 && (*instructionList_it)->get_isShared() == 1 && (*instructionList_it)->get_isUnique() == 1)
            {
               //update the address for later use
               (*instructionList_it)->set_physicalAddress(address);

               //swap the instructions and exit the loop -- NOTE assume that BEGIN is the first instruciton, otherwise lineNumber will be different
               tempInstruction = instructionList->at(lineNumber);
               instructionList->at(lineNumber) = *instructionList_it;
               *instructionList_it = tempInstruction;
               break;
            }
         }
      }
   }//--END LOADS

   //iterate through the list of conflicting STORES and mark the instructions in the synthetic instruction list
   for(conflictList_it = writeConflictList->begin(); conflictList_it != writeConflictList->end(); conflictList_it++)
   {
      address = conflictList_it->first;
      lineNumber = conflictList_it->second;
      isShared = conflictList_it->first & 1;                      //bitwise AND operation shows whether the address is shared or not

      #if defined(DEBUG)
      std::cerr << "S(" << (conflictList_it->first & 1) << ")-Address:  " << std::hex << address << "  Line:  " << std::dec << lineNumber << std::endl;
      #endif

      for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
      {
         //make sure the instruction is a STORE and has not been claimed
         if((*instructionList_it)->get_opCode() == iStore && (*instructionList_it)->get_physicalAddress() == 0)
         {
            if(isShared == 0  && (*instructionList_it)->get_isShared() == 0 && (*instructionList_it)->get_isUnique() == 1)
            {
               //update the address for later use
               (*instructionList_it)->set_physicalAddress(address);

               //swap the instructions and exit the loop -- NOTE assume that BEGIN is the first instruciton, otherwise lineNumber will be different
               tempInstruction = instructionList->at(lineNumber);
               instructionList->at(lineNumber) = *instructionList_it;
               *instructionList_it = tempInstruction;
               break;
            }
            else if(isShared == 1 && (*instructionList_it)->get_isShared() == 1 && (*instructionList_it)->get_isUnique() == 1)
            {
               //update the address for later use
               (*instructionList_it)->set_physicalAddress(address);

               //swap the instructions and exit the loop -- NOTE assume that BEGIN is the first instruciton, otherwise lineNumber will be different
               tempInstruction = instructionList->at(lineNumber);
               instructionList->at(lineNumber) = *instructionList_it;
               *instructionList_it = tempInstruction;
               break;
            }
         }
      }
   }//--END STORES
}
//END specifyMemory


/**
 * @ingroup Skin
 * 
 * @param instructionList 
 * @param readConflictList 
 * @param writeConflictList 
 */
void Skin::prioritizeMemory(std::deque < Instruction * > *instructionList, std::list< CONFLICT_PAIR >* readConflictList, std::list< CONFLICT_PAIR >* writeConflictList)
{
   /* Variables */
   UINT_32 lowest;
   UINT_32 currentDepth, newDepth;
   UINT_32 firstAllowedRead;
   UINT_32 firstAllowedWrite;

   Instruction *tempInstruction;

   std::deque< Instruction * >::iterator instructionList_it;
   std::deque< Instruction * >::iterator instructionListSearch_it;

   /* Processes */
   firstAllowedRead  = readConflictList->front().second;
   firstAllowedWrite = writeConflictList->front().second;

   //find first unique load
   for(std::list< CONFLICT_PAIR >::const_iterator conflictList_it = readConflictList->begin(); conflictList_it != readConflictList->end(); conflictList_it++)
   {
      if(conflictList_it->second < firstAllowedRead)
         firstAllowedRead = conflictList_it->second;
   }

   //find first unique store
   for(std::list< CONFLICT_PAIR >::const_iterator conflictList_it = writeConflictList->begin(); conflictList_it != writeConflictList->end(); conflictList_it++)
   {
      if(conflictList_it->second < firstAllowedWrite)
         firstAllowedWrite = conflictList_it->second;
   }

   for(instructionList_it = instructionList->begin(), currentDepth = 0; instructionList_it != instructionList->end(); instructionList_it++, currentDepth++)
   {
      //move misplaced loads
      if((*instructionList_it)->get_opCode() == iLoad && currentDepth < firstAllowedRead)
      {
         for(instructionListSearch_it = instructionList_it, newDepth = 0; instructionListSearch_it != instructionList->end(); instructionListSearch_it++, newDepth++)
         {
            if(newDepth > firstAllowedRead && (*instructionListSearch_it)->get_opCode() != iLoad && (*instructionListSearch_it)->get_opCode() != iStore && (*instructionListSearch_it)->get_opCode() != iCommitTX)
            {
               tempInstruction = instructionList->at(currentDepth);
               instructionList->at(currentDepth) = *instructionListSearch_it;
               *instructionListSearch_it = tempInstruction;
               break;
            }
         }
      }//--end load

      //move misplaced stores
      if((*instructionList_it)->get_opCode() == iStore && currentDepth < firstAllowedWrite)
      {
         for(instructionListSearch_it = instructionList_it, newDepth = 0; instructionListSearch_it != instructionList->end(); instructionListSearch_it++, newDepth++)
         {
            if(newDepth > firstAllowedRead && (*instructionListSearch_it)->get_opCode() != iLoad && (*instructionListSearch_it)->get_opCode() != iStore && (*instructionListSearch_it)->get_opCode() != iCommitTX)
            {
               tempInstruction = instructionList->at(currentDepth);
               instructionList->at(currentDepth) = *instructionListSearch_it;
               *instructionListSearch_it = tempInstruction;
               break;
            }
         }
      }//--end store
   }
}
//END prioritizeMemory


/**
 * @ingroup Skin
 * 
 * @note  So what if it isn't a word?
 * @param instructionList 
 * @param uniqueLoads 
 * @param uniqueStores 
 * @param useLoops 
 * @param iterations 
 * @param remainder 
 * @param loopSize 
 */
void Skin::conflictizeMemory(std::deque < Instruction * > *instructionList, UINT_32 uniqueLoads, UINT_32 uniqueStores, BOOL useLoops, UINT_32 iterations, UINT_32 remainder, UINT_32 loopSize)
{
   /* Variables */
   UINT_32 newMatchID;
   BOOL foundLoad = 0;
   BOOL foundStore = 0;
   Instruction *loadInstruction;
   Instruction *storeInstruction;
   std::deque< Instruction * >::iterator instructionList_it;

   UINT_32 newLoopSize;
   UINT_32 oldTotal;
   UINT_32 newIterations;
   UINT_32 newTotal;
   UINT_32 diff;

   /* Processes */
   for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
   {
      if((*instructionList_it)->get_isShared() == 1 && (*instructionList_it)->get_opCode() == iLoad)
      {
         foundLoad = 1;
         loadInstruction = (*instructionList_it);
         break;
      }
   }

   if(foundLoad == 1)
   {
      for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
      {
         if((*instructionList_it)->get_isShared() == 1 && (*instructionList_it)->get_opCode() == iStore)
         {
            foundStore = 1;
            storeInstruction = (*instructionList_it);
            break;
         }
      }
   }

   //we only pair instructions if there is both a shared load and a shared store
   //the instructions should also be reordered
   if(foundLoad == 1 && foundStore == 1)
   {
      //if there are enough unique references to ensure that the offset can be completely
      //unique, left shift the new ID by one for compare operation in Body and OR with 1
      newMatchID = matchID << 1;
      if(uniqueLoads > 1 && uniqueStores > 1)
         newMatchID = newMatchID | 1;

      loadInstruction->set_matchedInstruction(newMatchID);
      storeInstruction->set_matchedInstruction(newMatchID);

      matchID = matchID + 1;

      //move the load operation to the front of the transaction
      for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
      {
         if((*instructionList_it)->get_matchedInstruction() != -1 && (*instructionList_it)->get_opCode() == iLoad)
         {
            if(useLoops == 1)
            {
               instructionList->insert(instructionList->begin() + 2, (*instructionList_it));

               ///we now have an imbalance in the number of instructions per loop which needs to be corrected
               //if there is no remainder we need to completely rebalance the loop, otherwise we can do a replacement
               if(remainder > 0)
               {
                  std::deque< Instruction * >::iterator instructionList_it_2;
                  for(instructionList_it_2 = instructionList->begin(); instructionList_it_2 != instructionList->end(); instructionList_it_2++)
                  {
                     if((*instructionList_it_2)->get_opCode() == iEndLoop)
                     {
                        ++instructionList_it_2;
                        break;
                     }
                  }

                  (*instructionList_it) = (*instructionList_it_2);                  //replace the load with the first instruction after the loop
                  instructionList->erase(instructionList_it_2);                     //remove the replacement instruction
               }
               else
               {
                  instructionList->erase(instructionList_it);                       //remove the load from the loop body (this was unique and shared)

                  Instruction *instruction;
                  std::deque< Instruction * >::iterator instructionList_it_2;

                  //If the number of iterations is larger than the size of the loop, we need to calculate the new
                  //number of iterations based on the new loop size and then store the new number of iterations in
                  //the instruction stream and write out the difference. If the number of iterations is small than
                  //the loop size, then we can just write out the difference
                  if(iterations > loopSize)
                  {
                     newLoopSize = loopSize - 1;
                     oldTotal = loopSize * iterations;
                     newIterations = oldTotal / newLoopSize;
                     newTotal = newLoopSize * newIterations;
                     diff = oldTotal - newTotal;

                     for(instructionList_it_2 = instructionList->begin(); instructionList_it_2 != instructionList->end(); instructionList_it_2++)
                     {
                        if((*instructionList_it_2)->get_opCode() == iEndLoop)
                        {
                           if(iterations > loopSize)
                              (*instructionList_it_2)->set_iterations(newIterations);

                           ++instructionList_it_2;
                           break;
                        }
                     }
                  }
                  else
                  {
                     for(instructionList_it_2 = instructionList->begin(); instructionList_it_2 != instructionList->end(); instructionList_it_2++)
                     {
                        if((*instructionList_it_2)->get_opCode() == iEndLoop)
                        {
                           ++instructionList_it_2;
                           break;
                        }
                     }

                     diff = iterations;
                  }

                  //FIXME replace with all adds for the time being -- start count at 1 to account for displaced load
                  for(UINT_32 insCount = 1; insCount < diff; insCount++)
                  {
                     instruction = new Instruction(iALU);
                     instructionList->insert(instructionList_it_2, instruction);
                  }
               }

               break;
            }
            else
            {
               if((*instructionList_it)->get_matchedInstruction() != -1 && (*instructionList_it)->get_opCode() == iLoad)
               {
                  Instruction *tempInstruction = (*instructionList_it);
                  (*instructionList_it) = instructionList->at(1);
                  instructionList->at(1) = tempInstruction;
                  break;
               }
            }
         }
      }//---end load

      //move the store operation to the end of the transaction
      //the same rebalancing issues exist here -- see comment above
      for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
      {
         if((*instructionList_it)->get_matchedInstruction() != -1 && (*instructionList_it)->get_opCode() == iStore)
         {
            UINT_32 diff2;
            Instruction *storeInstruction = (*instructionList_it);
            std::deque< Instruction * >::iterator instructionList_it_2;

            if(useLoops == 1)
            {
               if(diff > 1)
               {
                  instructionList_it_2 = instructionList->end();
                  --instructionList_it_2;
                  --instructionList_it_2;

                  (*instructionList_it) = (*instructionList_it_2);                  //swtich the matched store with the 1st instruction after the loop
                  (*instructionList_it_2) = storeInstruction;
               }
               else
               {
                  if(newIterations > newLoopSize)
                  {
                     UINT_32 oldTotal2 = newLoopSize * newIterations;
                     UINT_32 newLoopSize2 = newLoopSize - 1;
                     UINT_32 newIterations2 = oldTotal2 / newLoopSize2;
                     UINT_32 newTotal2 = newLoopSize2 * newIterations2;
                     diff2 = oldTotal2 - newTotal2;

                     for(instructionList_it_2 = instructionList->begin(); instructionList_it_2 != instructionList->end(); instructionList_it_2++)
                     {
                        if((*instructionList_it_2)->get_opCode() == iEndLoop)
                        {
                           if(newIterations > newLoopSize)
                              (*instructionList_it_2)->set_iterations(newIterations2);

                           ++instructionList_it_2;
                           break;
                        }
                     }
                  }
                  else
                  {
                     for(instructionList_it_2 = instructionList->begin(); instructionList_it_2 != instructionList->end(); instructionList_it_2++)
                     {
                        if((*instructionList_it_2)->get_opCode() == iEndLoop)
                        {
                           ++instructionList_it_2;
                           break;
                        }
                     }

                     diff2 = newIterations;
                  }

                  //FIXME replace with all adds for the time being -- start count at 1 to account for displaced store
                  for(UINT_32 insCount = 1; insCount < diff2; insCount++)
                  {
                     Instruction *instruction = new Instruction(iALU);
                     instructionList->insert(instructionList_it_2, instruction);
                  }

                  instructionList->erase(instructionList_it);
                  instructionList_it_2 = instructionList->end();
                  instructionList->insert(instructionList_it_2 - 1, storeInstruction);
               }

               break;
            }
            else
            {
               (*instructionList_it) = instructionList->at(instructionList->size() - 2);
               instructionList->at(instructionList->size() - 2) = storeInstruction;
               break;
            }
         }
      }//---end store
   }
}
//END conflictizeMemory


/**
 * @ingroup Skin
 * 
 * @param instructionList 
 * @param conflictType 
 * @param cellType 
 * @param useLoops 
 */
void Skin::randomizeInstructionStream(std::deque < Instruction * > *instructionList, ConflictType conflictType, CellType cellType, UINT_32 sharedReads, UINT_32 sharedWrites, BOOL useLoops)
{
   /* Variables */
   std::deque< Instruction * >::iterator instructionList_it_begin;
   std::deque< Instruction * >::iterator instructionList_it_end;

   std::deque< Instruction * >::iterator instructionList_it_loop_begin;
   std::deque< Instruction * >::iterator instructionList_it_loop_end;

   /* Processes */
   //randomize the instrucitons in the loop body
   if(useLoops == 1)
   {
      //find the first instruction after the start of the loop
      for(std::deque< Instruction * >::iterator instructionList_it_2 = instructionList->begin(); instructionList_it_2 != instructionList->end(); instructionList_it_2++)
      {
         if((*instructionList_it_2)->get_opCode() == iBeginLoop)
         {
            instructionList_it_loop_begin = instructionList_it_2;
            break;
         }
      }

      //find the end of the loop body
      for(std::deque< Instruction * >::iterator instructionList_it_2 = instructionList->begin(); instructionList_it_2 != instructionList->end(); instructionList_it_2++)
      {
         if((*instructionList_it_2)->get_opCode() == iEndLoop)
         {
            instructionList_it_loop_end = instructionList_it_2;
            break;
         }
      }

      ++instructionList_it_loop_begin;
      --instructionList_it_loop_end;

//       for(UINT_32 brokenMixer = 0; brokenMixer < 5; brokenMixer++)
         std::random_shuffle(instructionList_it_loop_begin, instructionList_it_loop_end);
   }

   //randomize the instructions after the loop body
   if(cellType == Transactional)
   {
      //we want to ensure that a load is the 1st op and a write is the final op
      //however, someone let the program pass high conflict densities with zero reads
      //and zero writes so we need take that into account when finding the boundries
      if(conflictType == High)
      {
         //if this is after a loop, we need to start with the instruction after the loop terminator
         //if this is not a loop, we need to start with the instruction after the beginTx instruction
         //we need to ensure that the store at the end is not reordered
         if(useLoops == 1)
         {
            //find the end of the loop body
            for(std::deque< Instruction * >::iterator instructionList_it_2 = instructionList->begin(); instructionList_it_2 != instructionList->end(); instructionList_it_2++)
            {
               if((*instructionList_it_2)->get_opCode() == iEndLoop)
               {
                  instructionList_it_begin = instructionList_it_2;
                  break;
               }
            }

            if(sharedReads > 0)
               ++instructionList_it_begin;
         }
         else
         {
            instructionList_it_begin = instructionList->begin();
            ++instructionList_it_begin;
            ++instructionList_it_begin;
         }

         instructionList_it_end = instructionList->end();
         --instructionList_it_end;

         if(sharedWrites > 0)
            --instructionList_it_end;
      }
      else
      {
         //if the conflict density is low, we only care about the end of the loop and the end of the basic block
         if(useLoops == 1)
         {
            instructionList_it_begin = instructionList_it_loop_end;
            ++instructionList_it_begin;
            ++instructionList_it_begin;
         }
         else
         {
            instructionList_it_begin = instructionList->begin();
            ++instructionList_it_begin;
         }

         instructionList_it_end = instructionList->end();
         --instructionList_it_end;
      }
   }
   else
   {
      if(useLoops == 1)
      {
         instructionList_it_begin = instructionList_it_loop_end;
         ++instructionList_it_begin;
         ++instructionList_it_begin;
      }
      else
      {
         instructionList_it_begin = instructionList->begin();
      }

      instructionList_it_end = instructionList->end();
   }

//    for(UINT_32 brokenMixer = 0; brokenMixer < 5; brokenMixer++)
      std::random_shuffle(instructionList_it_begin, instructionList_it_end);
}
//END randomizeInstructionStream
