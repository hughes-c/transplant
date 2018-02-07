/**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009, 2010
 * @date    09/19/08
 * @brief   This is the implementation for the Body.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: Body
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////



#include "Body.h"

Body::Body() : basicBlockLabel(0), globalBase(0), maxGlobalOffset(0), privateBase(0), maxPrivateOffset(0), globalLoadBase(0), privateLoadBase(0), globalStoreBase(0), privateStoreBase(0), privLoad(0), privStore(0), sharedLoad(0), sharedStore(0), noOverlap(0), currentLockedOffset(-1)
{
}

Body::Body(Skin &skinIn) : basicBlockLabel(0), globalBase(0), maxGlobalOffset(0), privateBase(0), maxPrivateOffset(0), globalLoadBase(0), privateLoadBase(0), globalStoreBase(0), privateStoreBase(0), privLoad(0), privStore(0), sharedLoad(0), sharedStore(0), noOverlap(0), currentLockedOffset(-1), Skin(skinIn)
{
}

/**
 * @ingroup Body
 * @brief   Randomize memory locations
 *
 * @param min
 * @param max
 */
inline UINT_32 Body::randMemory(UINT_32 min, UINT_32 max)
{
   UINT_32 returnValue = uniformIntRV(min, max);

   if(returnValue > 0)
      return returnValue - 1;
   else
      return returnValue;
}

/**
 * @ingroup Body
 * @brief   Writes the final output
 *
 * @param
 */
void Body::writeProgram(void)
{
   /* Variables */
   BOOL const barrier_per_transaction = 0;                                             //should there be a barrier before each transaction?
   BOOL const reset_mem_per_cell = config->read<BOOL>("Global", "resetPerCell");       //should memory be reset at the start of each cell?
   BOOL const barrier_per_thread = config->read<BOOL>("Global", "barrierPerThread");   //should there be a barrier at the start of each thread?

   UINT_32 count = 0;
   UINT_32 instructionID = 0;
   std::deque< Instruction * > *instructionList;
   std::deque< Instruction * >::iterator instructionList_it;

   std::string currentFileName;
   std::string fileName = "output/";
   fileName = fileName + config->read<string>("Global", "fileName");

   /* Processes */
   std::cout << "Writing synthetic program to " << fileName << " -- ";
   for(THREAD_ID threadID = 0; threadID < numThreads; threadID++)
   {
      std::cout << "T" << threadID << "..." << std::flush;

      //set the working list equal to the current thread's instruction list
      instructionList = perThread_instructionList[threadID];

      //reset the label count for each thread
      basicBlockLabel = 0;

      //reset the base of the private memory region
      privateBase = globalBase = globalLoadBase = globalStoreBase = privateLoadBase = privateStoreBase = 0;
      privLoad = privStore = 0;
      sharedLoad = sharedStore = 0;

      //new instantiation of map to match address offsets in each cell
      addressMatch = new std::map< UINT_32, ADDRESS_INT >;

      //new instantiation of vectors containing a list of the 'used' address offsets
      globalLoadOffsetList   = new std::vector< UINT_32 > (1, 0);
      privateLoadOffsetList  = new std::vector< UINT_32 > (1, 0);

      globalStoreOffsetList  = new std::vector< UINT_32 > (1, 0);
      privateStoreOffsetList = new std::vector< UINT_32 > (1, 0);

      currentLockedOffset = -1;

      //set up the new name -- you know 'cause naming is important
      if(threadID == 0)
         currentFileName = fileName + ".c";
      else
         currentFileName = fileName + "_" + Instruction::IntToString(threadID) + ".h";

      //open the file
      std::ofstream outputFile(currentFileName.c_str(), std::ios::trunc);   //open a file for writing (append the current contents)
      if(!outputFile)                                                //check to be sure file is open
         std::cerr << "Error opening file.\n";

      //set up the head
      if(threadID == 0)
         headerGen(outputFile);
      else
         funcHeaderGen(threadID, outputFile);

      //insert specials
      if(barrier_per_thread == 1)
      {
         insertBarrier(outputFile, numThreads);
      }

      beginProgamIterations(outputFile, threadID);

      if(config->read<int>("Global", "numBarriers") > 1)
      {
         insertBarrier(outputFile, numThreads);
      }

      ///put in the guts
      //Iterate through the instruction list and write out each instruction
      for(instructionList_it = instructionList->begin(); instructionList_it != instructionList->end(); instructionList_it++)
      {
         OperandList operandList;

         if((*instructionList_it)->get_opCode() == iBJ)
         {

            translateInstruction(threadID, (*instructionList_it), operandList);
            writeInstruction(outputFile, (*instructionList_it), operandList);
            writeLabel(outputFile, threadID);

            //starting a new cell, reset private flags
            privLoad = privStore = 0;

            //only do this if memory is reset at the start of each cell
            if(reset_mem_per_cell == 1)
            {
               sharedLoad = sharedStore = 0;
               globalLoadOffsetList->erase(globalLoadOffsetList->begin() + 1, globalLoadOffsetList->end());
               globalStoreOffsetList->erase(globalStoreOffsetList->begin() + 1, globalStoreOffsetList->end());
            }

            privateLoadOffsetList->erase(privateLoadOffsetList->begin() + 1, privateLoadOffsetList->end());
            privateStoreOffsetList->erase(privateStoreOffsetList->begin() + 1, privateStoreOffsetList->end());
         }
         else if((*instructionList_it)->get_opCode() == syncBarrier)
         {
            insertBarrier(outputFile, numThreads);
         }
         else if((*instructionList_it)->get_opCode() == iBeginTX)
         {
            if(barrier_per_transaction == 1)
            {
               insertBarrier(outputFile, numThreads);
            }
            startTransSection(outputFile, 0);

            //If the conflict model is high we need to ensure there are no overlapping l/s between the pair
            if((*instructionList_it)->get_conflictModel() == High)
               noOverlap = 1;
            else
               noOverlap = 0;
         }
         else if((*instructionList_it)->get_opCode() == iCommitTX)
         {
            noOverlap = 0;
            endTransSection(outputFile, 0);
         }
         else if((*instructionList_it)->get_opCode() == iInitLoop)
         {
            if((*instructionList_it)->get_subCode() == CellLoop)
               initCellLoopSection(outputFile);
         }
         else if((*instructionList_it)->get_opCode() == iBeginLoop)
         {
            if((*instructionList_it)->get_subCode() == CellLoop)
               beginCellLoopSection(outputFile, threadID);
            else if((*instructionList_it)->get_subCode() == MultiCellLoop)
            {
               instructionID = (*instructionList_it)->get_instructionID();
               beginMultiCellLoopSection(outputFile, threadID, instructionID);
            }
            else
            {
               instructionID = (*instructionList_it)->get_instructionID();
               beginBlockLoopSection(outputFile, threadID, instructionID);
            }
         }
         else if((*instructionList_it)->get_opCode() == iEndLoop)
         {
            if((*instructionList_it)->get_subCode() == CellLoop)
               endCellLoopSection(outputFile, threadID, (*instructionList_it)->get_iterations());
            else if((*instructionList_it)->get_subCode() == MultiCellLoop)
               endMultiCellLoopSection(outputFile, threadID, instructionID, (*instructionList_it)->get_iterations());
            else
               endBlockLoopSection(outputFile, threadID, instructionID, (*instructionList_it)->get_iterations());
         }
         else
         {
            translateInstruction(threadID, (*instructionList_it), operandList);
            writeInstruction(outputFile, (*instructionList_it), operandList);
         }
      }

      //insert specials
      endProgamIterations(outputFile);

      //set up the tail
      if(threadID == 0)
      {
         insertWait(outputFile, numThreads);
         trailerGen(outputFile);
      }
      else
      {
         funcTrailerGen(threadID, outputFile);
      }

      //clean up
      outputFile.close();

      delete addressMatch;

      delete globalLoadOffsetList;
      delete privateLoadOffsetList;

      delete globalStoreOffsetList;
      delete privateStoreOffsetList;
   }

   std::cout << "It's ALIVE!" << std::endl;
}

/**
 * @ingroup Body
 * @brief   Determines asm from high-level specification
 *
 * @param threadID
 * @param instructionIn
 * @param operandList
 */
void Body::translateInstruction(THREAD_ID threadID, Instruction *instructionIn, OperandList &operandList)
{
   /* Variables */
   UINT_32 globalOffset;
   UINT_32 privateOffset;

   /* Processes */
   if(instructionIn->get_opCode() == iALU)
   {
      operandList.rs = "%1";
      operandList.rt = "%2";
      operandList.rd = "%0";

      operandList.rs_variable = Instruction::getIntVariable(instructionIn->get_rs());
      operandList.rt_variable = Instruction::getIntVariable(instructionIn->get_rt());
      operandList.rd_variable = Instruction::getIntVariable(instructionIn->get_rd());
   }
   else if(instructionIn->get_opCode() == fpALU)
   {
      operandList.rs = "%1";
      operandList.rt = "%2";
      operandList.rd = "%0";

      operandList.rs_variable = Instruction::getFPVariable(instructionIn->get_rs());
      operandList.rt_variable = Instruction::getFPVariable(instructionIn->get_rt());
      operandList.rd_variable = Instruction::getFPVariable(instructionIn->get_rd());
   }
   else if(instructionIn->get_opCode() == iLoad)
   {
      std::map< UINT_32, ADDRESS_INT >::iterator matchIterator;

      if(instructionIn->get_isUnique() == 1)
      {
         ///NOTE Stopped here
         if(instructionIn->get_isShared() == 1 && sharedLoad == 0 && instructionIn->get_physicalAddress() != 0)
         {
            sharedLoad = 1;

            ///FIXME load is always seen first -- especially not the case with SPECIFIED
            //check to see if we need to match an offset and if there is 
            if(instructionIn->get_matchedInstruction() != -1 && instructionIn->get_matchedInstruction() & 1 == 0)
               globalLoadOffsetList->erase(globalLoadOffsetList->begin());
            else
               globalLoadBase = globalStoreBase = 0;
         }
         else if(instructionIn->get_isShared() == 1 && sharedLoad == 0)
         {
            sharedLoad = 1;

            ///FIXME load is always seen first -- especially not the case with SPECIFIED
            //check to see if we need to match an offset and if there is 
            if(instructionIn->get_matchedInstruction() != -1 && instructionIn->get_matchedInstruction() & 1 == 0)
               globalLoadOffsetList->erase(globalLoadOffsetList->begin());
            else
               globalLoadBase = globalStoreBase = 0;
         }
         else if(instructionIn->get_isShared() == 1)
         {
            globalLoadBase = globalLoadBase + CACHE_LINE;
            if(globalLoadBase > _15_BIT_RANGE)
            {
               std::cerr << "\nWARNING -- Load (" << globalLoadBase << std::dec << ") exceeded 16-bit address. Reset global load and store offsets." << std::endl;
               globalLoadBase = globalStoreBase = 0;
            }

            //If we want to prevent interleaving overlap, we need to make sure that loads and stores do not touch
            if(noOverlap == 1)
            {
               BOOL searching = 0;
               do
               {
                  for(std::vector< UINT_32 >::const_iterator temp_it = globalStoreOffsetList->begin(); temp_it != globalStoreOffsetList->end(); temp_it++)
                  {
                     if(globalLoadBase == *temp_it)
                     {
                        searching = 1;
                        globalLoadBase = globalLoadBase + CACHE_LINE;
                        break;
                     }
                     else
                        searching = 0;
                  }
               }while(searching == 1);
            }

            //check to see if we need to match an offset -- if not, then we we want to add this to the pool
            if(globalLoadBase >= _15_BIT_RANGE - 32) //-save one spot for stores
               globalLoadBase = 0;
            else if(instructionIn->get_matchedInstruction() == -1 || instructionIn->get_matchedInstruction() & 1 == 1)
               globalLoadOffsetList->push_back(globalLoadBase);
         }
         else if(instructionIn->get_isShared() == 0 && privLoad == 0)
         {
            privLoad = 1;

            //check to see if we need to match an offset
            if(instructionIn->get_matchedInstruction() != -1 && instructionIn->get_matchedInstruction() & 1 == 0)
               privateLoadOffsetList->erase(privateLoadOffsetList->begin());
            else
               privateLoadBase = 0;
         }
         else if(privLoad == 1)
         {
            privateLoadBase = privateLoadBase + CACHE_LINE;
            if(privateLoadBase > _15_BIT_RANGE)
            {
               std::cerr << "\nWARNING -- Load exceeded 16-bit address. Reset private load offset." << std::endl;
               privateLoadBase = 0;
            }

            //check to see if we need to match an offset -- if not, then we we want to add this to the pool
            if(instructionIn->get_matchedInstruction() == -1 || instructionIn->get_matchedInstruction() & 1 == 1)
               privateLoadOffsetList->push_back(privateLoadBase);
         }

         //if we're out of the bounds for the memory pool, exit
         if(globalLoadBase > MAX_MEM || privateLoadBase > MAX_MEM)
         {
            std::cerr << "Memory Bound (" << globalLoadBase << " - " << privateLoadBase << ")" << std::endl;
            exit(1);
         }

         globalOffset = globalLoadBase;
         privateOffset = privateLoadBase;
      }
      else
      {
         UINT_32 globalRef = randMemory(0, globalLoadOffsetList->size());
         UINT_32 privateRef = randMemory(0, privateLoadOffsetList->size());

         globalOffset = (*globalLoadOffsetList)[globalRef];
         privateOffset = (*privateLoadOffsetList)[privateRef];;
      }

      if(instructionIn->get_isShared() == 1)
      {
         //check to see if we need to match an offset
         if(instructionIn->get_matchedInstruction() != -1)
         {
            BOOL unique;

            boost::tie(matchIterator, unique) = addressMatch->insert(std::make_pair(instructionIn->get_matchedInstruction(), globalOffset));
            if(unique == 0)
               globalOffset = (*addressMatch)[instructionIn->get_matchedInstruction()];

            currentLockedOffset = globalOffset;

            #if defined(VERBOSE)
            std::cout << "(" << threadID << ")L-Matched with:  " << instructionIn->get_matchedInstruction() << " at " <<  globalOffset << "\n";
            #endif
         }

         operandList.rs = Instruction::IntToString(globalOffset) + "(%1)";
         operandList.rs_variable =  "r\"(s_data_out_int)";
      }
      else
      {
         //check to see if we need to match an offset
         if(instructionIn->get_matchedInstruction() != -1)
         {
            BOOL unique;

            boost::tie(matchIterator, unique) = addressMatch->insert(std::make_pair(instructionIn->get_matchedInstruction(), privateOffset));
            if(unique == 0)
               privateOffset = (*addressMatch)[instructionIn->get_matchedInstruction()];

            #if defined(VERBOSE)
            std::cout << "(" << threadID << ")L-Matched with:  " << instructionIn->get_matchedInstruction() << " at " <<  privateOffset << "\n";
            #endif
         }

         operandList.rs = Instruction::IntToString(privateOffset) + "(%1)";
         operandList.rs_variable =  "r\"(data_out_int)";
      }

      operandList.rt = "";
      operandList.rd = "%0";

      operandList.rd_variable = Instruction::getIntVariable(instructionIn->get_rd());
   }
   else if(instructionIn->get_opCode() == iStore)
   {
      std::map< UINT_32, ADDRESS_INT >::iterator matchIterator;

      if(instructionIn->get_isUnique() == 1)
      {
         if(instructionIn->get_isShared() == 1 && sharedStore == 0)
         {
            sharedStore = 1;

            //check to see if we need to match an offset
            if(instructionIn->get_matchedInstruction() != -1 && instructionIn->get_matchedInstruction() & 1 == 0)
            {
               globalStoreOffsetList->erase(globalStoreOffsetList->begin());
            }
            else
            {
               for(std::map< UINT_32, ADDRESS_INT >::iterator matchIterator = addressMatch->begin(); matchIterator != addressMatch->end(); matchIterator++)
               {
                  if(matchIterator->second == 0)
                  {
                     globalStoreBase = globalStoreBase + CACHE_LINE;

                     //If we want to prevent interleaving overlap, we need to make sure that loads and stores do not touch
                     if(noOverlap == 1)
                     {
                        BOOL searching = 0;
                        UINT_32 lastOffset = 0;
                        do
                        {
                           for(std::vector< UINT_32 >::const_iterator temp_it = globalLoadOffsetList->begin(); temp_it != globalLoadOffsetList->end(); temp_it++)
                           {
                              //Looking for offset match + out of range + current conflict offset
                              if(globalStoreBase == *temp_it || globalStoreBase == currentLockedOffset)
                              {
                                 searching = 1;
                                 lastOffset = globalStoreBase;
                                 globalStoreBase = globalStoreBase + CACHE_LINE;

                                 if(globalStoreBase > _15_BIT_RANGE)
                                 {
                                    std::cerr << "\nWARNING -- Store (" << globalStoreBase << std::dec << ") exceeded 16-bit address. Reset global load and store offsets." << std::endl;
                                    globalLoadBase = globalStoreBase = 0;
                                 }

                                 break;
                              }
                              else
                                 searching = 0;

//                               //Early exit if we are at the memory boundry
//                               if(lastOffset == globalStoreBase && globalStoreBase == _15_BIT_RANGE)
//                                  searching = 0;
                           }
                        }while(searching == 1);
                     }

                     globalStoreOffsetList->push_back(globalStoreBase);
                     break;
                  }
                  else
                     globalStoreBase = 0;
               }
            }
         }
         else if(instructionIn->get_isShared() == 1)
         {
            globalStoreBase = globalStoreBase + CACHE_LINE;
            if(globalStoreBase > _15_BIT_RANGE)
            {
               std::cerr << "\nWARNING -- Store (" << globalStoreBase << std::dec << ") exceeded 16-bit address. Reset global load and store offsets." << std::endl;
               globalLoadBase = globalStoreBase = 0;
            }

            //If we want to prevent interleaving overlap, we need to make sure that loads and stores do not touch
            if(noOverlap == 1)
            {
               BOOL searching = 0;
               do
               {
                  for(std::vector< UINT_32 >::const_iterator temp_it = globalLoadOffsetList->begin(); temp_it != globalLoadOffsetList->end(); temp_it++)
                  {
                     if(globalStoreBase == *temp_it || globalStoreBase == currentLockedOffset)
                     {
                        searching = 1;
                        globalStoreBase = globalStoreBase + CACHE_LINE;
                        break;
                     }
                     else
                        searching = 0;
                  }
               }while(searching == 1);
            }

            //check to see if we need to match an offset -- if not, then we we want to add this to the pool
            if(instructionIn->get_matchedInstruction() == -1 || instructionIn->get_matchedInstruction() & 1 == 1)
               globalStoreOffsetList->push_back(globalStoreBase);
         }
         else if(instructionIn->get_isShared() == 0 && privStore == 0)
         {
            privStore = 1;

            //check to see if we need to match an offset
            if(instructionIn->get_matchedInstruction() != -1 && instructionIn->get_matchedInstruction() & 1 == 0)
               privateStoreOffsetList->erase(privateStoreOffsetList->begin());
            else
               privateStoreBase = 0;
         }
         else if(privStore = 1)
         {
            privateStoreBase = privateStoreBase + CACHE_LINE;
            if(privateStoreBase > _15_BIT_RANGE)
            {
               std::cerr << "\nWARNING -- Exceeded 16-bit address. Reset private store offsets." << std::endl;
               privateStoreBase = 0;
            }

            //check to see if we need to match an offset -- if not, then we we want to add this to the pool
            if(instructionIn->get_matchedInstruction() == -1 || instructionIn->get_matchedInstruction() & 1 == 1)
               privateStoreOffsetList->push_back(privateStoreBase);
         }

//          //if we're out of the bounds for the memory pool, exit
//          if(globalLoadBase > MAX_MEM || privateLoadBase > MAX_MEM)
//          {
//             std::cerr << "Memory Bound" << std::endl;
//             exit(1);
//          }

         globalOffset = globalStoreBase;
         privateOffset = privateStoreBase;
      }
      else
      {
         UINT_32 globalRef = randMemory(0, globalStoreOffsetList->size());
         UINT_32 privateRef = randMemory(0, privateStoreOffsetList->size());

         globalOffset = (*globalStoreOffsetList)[globalRef];
         privateOffset = (*privateStoreOffsetList)[privateRef];
      }

      if(instructionIn->get_isShared() == 1)
      {
         //check to see if we need to match an offset
         if(instructionIn->get_matchedInstruction() != -1)
         {
            BOOL unique;

            boost::tie(matchIterator, unique) = addressMatch->insert(std::make_pair(instructionIn->get_matchedInstruction(), globalOffset));
            if(unique == 0)
               globalOffset = (*addressMatch)[instructionIn->get_matchedInstruction()];

            #if defined(VERBOSE)
            std::cout << "(" << threadID << ")S-Matched with:  " << instructionIn->get_matchedInstruction() << " at " <<  globalOffset << "\n";
            #endif
         }

         operandList.rt = Instruction::IntToString(globalOffset) + "(%0)";
         operandList.rt_variable =  "r\"(s_data_out_int)";
      }
      else
      {
         //check to see if we need to match an offset
         if(instructionIn->get_matchedInstruction() != -1)
         {
            BOOL unique;

            boost::tie(matchIterator, unique) = addressMatch->insert(std::make_pair(instructionIn->get_matchedInstruction(), privateOffset));
            if(unique == 0)
               privateOffset = (*addressMatch)[instructionIn->get_matchedInstruction()];

            #if defined(VERBOSE)
            std::cout << "(" << threadID << ")S-Matched with:  " << instructionIn->get_matchedInstruction() << " at " <<  privateOffset << "\n";
            #endif
         }

         operandList.rt = Instruction::IntToString(privateOffset) + "(%0)";
         operandList.rt_variable =  "r\"(data_out_int)";
      }

      operandList.rs = "";
      operandList.rd = "%1";

      operandList.clobberList = operandList.clobberList + "\"memory\"";

      operandList.rd_variable = Instruction::getIntVariable(instructionIn->get_rd());
   }
   else if(instructionIn->get_opCode() == iBJ)
   {
      basicBlockLabel = basicBlockLabel + 1;

      operandList.rs = "";
      operandList.rt = "";
      operandList.rd =  "I" + Instruction::IntToString(threadID) + "_" + Instruction::IntToString(basicBlockLabel) + "_";

      operandList.rs_variable = "";
      operandList.rt_variable = "";
      operandList.rd_variable = "";
   }
}

/**
 * @ingroup Body
 * @brief   Instructions are written individually
 *
 * @param outputFile
 * @param instructionIn
 * @param operandList
 */
void Body::writeInstruction(std::ofstream &outputFile, Instruction *instructionIn, OperandList &operandList)
{
   /* Variables */

   /* Processes */
   outputFile << "   __asm__ __volatile__ ( \"";

   if(instructionIn->get_opCode() == iALU)
   {
//       outputFile << "add";
      outputFile << "xor";
      if(operandList.rd != "")
         outputFile << " "  << operandList.rd;
      if(operandList.rs != "")
         outputFile << ", " << operandList.rs;
      if(operandList.rt != "")
         outputFile << ", " << operandList.rt;

      outputFile << std::setw(6) << "\"\t :";
   }
   else if(instructionIn->get_opCode() == fpALU)
   {
      outputFile << "add.d";
      if(operandList.rd != "")
         outputFile << " "  << operandList.rd;
      if(operandList.rs != "")
         outputFile << ", " << operandList.rs;
      if(operandList.rt != "")
         outputFile << ", " << operandList.rt;

      outputFile << std::setw(6) << "\"\t :";
   }
   else if(instructionIn->get_opCode() == iLoad)
   {
      outputFile << "lw";
      if(operandList.rd != "")
         outputFile << " "  << operandList.rd;
      if(operandList.rs != "")
         outputFile << ", " << operandList.rs;
      if(operandList.rt != "")
         outputFile << ", " << operandList.rt;

      outputFile << std::setw(6) << "\"\t :";
   }
   else if(instructionIn->get_opCode() == iBJ)
   {
      outputFile << "b";
      if(operandList.rd != "")
         outputFile << " "  << operandList.rd;
      if(operandList.rs != "")
         outputFile << ", " << operandList.rs;
      if(operandList.rt != "")
         outputFile << ", " << operandList.rt;

      outputFile << std::setw(6) << "\"\t :";
   }

   if(instructionIn->get_opCode() == iStore)
   {
      outputFile << "sw";
      if(operandList.rd != "")
         outputFile << " "  << operandList.rd;
      if(operandList.rs != "")
         outputFile << ", " << operandList.rs;
      if(operandList.rt != "")
         outputFile << ", " << operandList.rt;

      outputFile << std::setw(6) << "\"\t :";

      outputFile << " \"=" << operandList.rt_variable;

      outputFile << " :";
      outputFile << " \""  << operandList.rd_variable;
   }
   else
   {
      if(operandList.rd != "" && instructionIn->get_opCode() != iBJ)
         outputFile << " \"=" << operandList.rd_variable;

      outputFile << " :";
      if(operandList.rs != "" && operandList.rs_variable != "")
         outputFile << " \""  << operandList.rs_variable;
      if(operandList.rt != "" && operandList.rs == "")
         outputFile << " \""  << operandList.rt_variable;
      else if(operandList.rt != "")
         outputFile << ", \"" << operandList.rt_variable;
   }

   if(operandList.clobberList.size() > 0)
      outputFile << " : " << operandList.clobberList;

   outputFile << " );";

   outputFile << std::endl;
}

/**
 * @ingroup Body
 *
 * @param outputFile 
 * @param threadID 
 */
inline void Body::writeLabel(std::ofstream &outputFile, THREAD_ID threadID)
{
   outputFile << "\n";
   outputFile << "   __asm__ __volatile__ (\"";
   outputFile << "I" + Instruction::IntToString(threadID) + "_" + Instruction::IntToString(basicBlockLabel) + "_:\");";
   outputFile << "\n";
}

/**
 * @ingroup Body
 *
 * @param outputFile 
 * @param numThreads 
 */
inline void Body::insertBarrier(std::ofstream &outputFile, UINT_32 numThreads)
{
   outputFile << "   sesc_barrier(&" << "paramBarr" << ", " << numThreads << ");\n";
   outputFile << std::endl;
}

/**
 * @ingroup Body
 *
 * @param outputFile 
 * @param numThreads 
 */
inline void Body::insertWait(std::ofstream &outputFile, UINT_32 numThreads)
{
   outputFile << "\n";
   for(UINT_32 counter = 0; counter < numThreads; counter++)
   {
      outputFile << "   sesc_wait();\n";
   }
   outputFile << std::endl;

}

/**
 * @ingroup Body
 *
 * @param outputFile 
 * @param transID 
 */
inline void Body::startTransSection(std::ofstream &outputFile, TX_ID transID)
{
   outputFile << "   BEGIN_TRANSACTION(" << std:: hex << transID << ");\n" << std::dec;
}

/**
 * @ingroup Body
 *
 * @param outputFile 
 * @param transID 
 */
inline void Body::endTransSection(std::ofstream &outputFile, TX_ID transID)
{
   outputFile << "   COMMIT_TRANSACTION(" << std:: hex << transID << ");\n" << std::dec;
}

/**
 * @ingroup Body
 *
 * @param outputFile 
 */
void Body::initCellLoopSection(std::ofstream &outputFile)
{
   outputFile << "   __asm__ __volatile__ ( \"move %0, $0\"" << std::setw(33) << ": \"=r\"(cell_counter) );";
   outputFile << "\n";
}

/**
 * @ingroup Body
 *
 * @param outputFile 
 * @param threadID 
 */
void Body::beginCellLoopSection(std::ofstream &outputFile, THREAD_ID threadID)
{
   outputFile << "   __asm__ __volatile__ (\"";
   outputFile << "LOOP_START_" + Instruction::IntToString(threadID) + "_" + Instruction::IntToString(basicBlockLabel) + "_:\");";
   outputFile << "\n";
}

/**
 * @ingroup Body
 * 
 * @param outputFile 
 * @param threadID 
 * @param iterations 
 */
void Body::endCellLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 iterations)
{
   outputFile << "   __asm__ __volatile__ ( \"addi %0, %1, 1\"" << std::setw(50) << ": \"=r\"(cell_counter) :\"r\"(cell_counter) );\n";
   outputFile << "   __asm__ __volatile__ ( \"slti %0, %1, " << iterations << "\"" << std::setw(46) << ": \"=r\"(r_out_t0) :\"r\"(cell_counter) );\n";
   outputFile << "   __asm__ __volatile__ ( \"bne  %0, $0, ";
   outputFile << "LOOP_START_" + Instruction::IntToString(threadID) + "_" + Instruction::IntToString(basicBlockLabel) + "_";
   outputFile << "\" : :\"r\"(r_out_t0) );\n";
}

/**
 * @ingroup Body
 *
 * @param outputFile 
 * @param threadID 
 * @param loopID 
 */
void Body::beginMultiCellLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 loopID)
{
   outputFile << "   __asm__ __volatile__ (\"";
   outputFile << "LOOP_START_" + Instruction::IntToString(threadID) + "_" + Instruction::IntToString(loopID) + "_" + Instruction::IntToString(basicBlockLabel) + "_:\");";
   outputFile << "\n";
}

/**
 * @ingroup Body
 *
 * @param outputFile 
 * @param threadID 
 * @param loopID 
 * @param iterations 
 */
void Body::endMultiCellLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 loopID, UINT_32 iterations)
{
   outputFile << "   __asm__ __volatile__ ( \"addi %0, %1, 1\"" << std::setw(50) << ": \"=r\"(cell_counter) :\"r\"(cell_counter) );\n";
   outputFile << "   __asm__ __volatile__ ( \"slti %0, %1, " << iterations << "\"" << std::setw(46) << ": \"=r\"(r_out_t0) :\"r\"(cell_counter) );\n";
   outputFile << "   __asm__ __volatile__ ( \"bne  %0, $0, ";
   outputFile << "LOOP_START_" + Instruction::IntToString(threadID) + "_" + Instruction::IntToString(loopID) + "_" + Instruction::IntToString(basicBlockLabel) + "_";
   outputFile << "\" : :\"r\"(r_out_t0) );\n";
}


/**
 * @ingroup Body
 * 
 * @param outputFile 
 * @param threadID 
 * @param loopID 
 */
void Body::beginBlockLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 loopID)
{
   outputFile << "   __asm__ __volatile__ ( \"move %0, $0\"" << std::setw(34) << ": \"=r\"(block_counter) );";
   outputFile << "\n";

   outputFile << "   __asm__ __volatile__ (\"";
   outputFile << "BLOCK_START_" + Instruction::IntToString(threadID) + "_" + Instruction::IntToString(loopID) + "_:\");";
   outputFile << "\n\n";
}


/**
 * @ingroup Body
 * 
 * @param outputFile 
 * @param threadID 
 * @param loopID 
 * @param iterations 
 */
void Body::endBlockLoopSection(std::ofstream &outputFile, THREAD_ID threadID, UINT_32 loopID, UINT_32 iterations)
{
   outputFile << "   __asm__ __volatile__ ( \"addi %0, %1, 1\"" << std::setw(52) << ": \"=r\"(block_counter) :\"r\"(block_counter) );\n";
   outputFile << "   __asm__ __volatile__ ( \"slti %0, %1, " << iterations << "\"" << std::setw(47) << ": \"=r\"(r_out_t0) :\"r\"(block_counter) );\n";
   outputFile << "   __asm__ __volatile__ ( \"bne  %0, $0, ";
   outputFile << "BLOCK_START_" + Instruction::IntToString(threadID) + "_" + Instruction::IntToString(loopID) + "_";
   outputFile << "\" : :\"r\"(r_out_t0) );\n";
}


/**
 * @ingroup Body
 * 
 * @param outputFile 
 * @param threadID 
 */
inline void Body::beginProgamIterations(std::ofstream &outputFile, THREAD_ID threadID)
{
   outputFile << "   for(counter = 0; counter < " << "LOOP_" << threadID << "; counter++)";
   outputFile << "\n";
   outputFile << "   {";
   outputFile << "\n";
// outputFile << "   fprintf(stderr, \"Thread " << threadID << " iteration %d\\n\", counter);\n";
// outputFile << "   fflush(stderr);\n";
}

/**
 * @ingroup Body
 * 
 * @param outputFile 
 */
inline void Body::endProgamIterations(std::ofstream &outputFile)
{
   outputFile << "   }";
   outputFile << "\t//end LOOP";
   outputFile << "\n";
}

/**
 * @ingroup Body
 *
 * @param outputFile 
**/
void Body::headerGen(std::ofstream &outputFile)
{
   /* Variables */
   UINT_32 maxInstructions = 0;
   UINT_32 memSize = MEM_REGION;

   /* Processes */
   outputFile << "//  This file was automatically generated.\n//  Do not edit this file.\n//\n//\n";
   outputFile << "/// @file param_prog.c\n";
   outputFile << "/// @author IDEAL Lab, University of Florida\n";
   outputFile << "/// @author Clay Hughes, James Poe, and Tao Li\n";
   outputFile << "//\n// Copyright: See COPYING file that comes with this distribution\n//\n///////////////////////////////////////////////////////////////////////////////////\n";

   outputFile << "#include <stdio.h>\n";
   outputFile << "#include <stdint.h>\n";
   outputFile << "#include <stdlib.h>\n";
   outputFile << "#include \"sescapi.h\"\n\n";

   for(UINT_32 threadCounter = 0; threadCounter < numThreads; threadCounter++)
   {
      outputFile << "#define LOOP_" << Instruction::IntToString(threadCounter) << " ";
      if(config->keyExists( "Global" , "numLoops"))
         outputFile << config->read<unsigned int>( "Global" , "numLoops");
      else
         outputFile << "1";
      outputFile << "\n";
   }
   outputFile << "\n";

   outputFile << "#define BEGIN_TRANSACTION(n) do {\t\t\t\\\n";
   outputFile << "        __asm__ __volatile__ (\".word 0x70000000+\" #n);\t\\\n";
   outputFile << "} while (0)\n\n";

   outputFile << "#define COMMIT_TRANSACTION(n) do {\t\t\t\\\n";
   outputFile << "        __asm__ __volatile__ (\".word 0x7C000000+\" #n);\t\\\n";
   outputFile << "} while (0)\n";

   outputFile << "\n/* Set Barrier */\n";
   outputFile << "sbarrier_t paramBarr;\n";

   outputFile << "\n/* Initialize Shared Memory Region */\n";
   outputFile << "int* shared_memInt;\n";
   outputFile << "register int s_data_out_int asm(\"22\");\n";
//    outputFile << "float* shared_memFloat;\n";
//    outputFile << "register int s_data_out_float asm(\"23\");\n";
   outputFile << "\n";

   outputFile << "int s_data_out_int_base;\n";
//    outputFile << "int s_data_out_float_base;\n\n";

   //includes for thread header files
   string rootName = config->read<string>("Global", "fileName");
   for(UINT_32 threadCounter = 1; threadCounter < numThreads; threadCounter++)
   {
      outputFile << "#include \"" << rootName << "_" << threadCounter << ".h\"";
      outputFile << "\n";
   }

   outputFile << "\n\nint main()\n{\n";							// --- main

   outputFile << "   register int r_out_t0 asm(\"8\");\n";
   outputFile << "   register int r_out_t1 asm(\"9\");\n";
   outputFile << "   register int r_out_t2 asm(\"10\");\n";
   outputFile << "   register int r_out_t3 asm(\"11\");\n";
   outputFile << "   register int r_out_t4 asm(\"12\");\n";
   outputFile << "   register int r_out_t5 asm(\"13\");\n";

   outputFile << "   register int r_out_s0 asm(\"16\");\n";
   outputFile << "   register int r_out_s1 asm(\"17\");\n";
   outputFile << "   register int r_out_s2 asm(\"18\");\n";
   outputFile << "   register int r_out_s3 asm(\"19\");\n";
   outputFile << "   register int r_out_s4 asm(\"20\");\n";
   outputFile << "   register int r_out_s5 asm(\"21\");\n";

   outputFile << "   register int r_out_f2 asm(\"$f2\");\n";
   outputFile << "   register int r_out_f4 asm(\"$f4\");\n";
   outputFile << "   register int r_out_f6 asm(\"$f6\");\n";
   outputFile << "   register int r_out_f8 asm(\"$f8\");\n";
   outputFile << "   register int r_out_f10 asm(\"$f10\");\n";
   outputFile << "   register int r_out_f12 asm(\"$f12\");\n";

   outputFile << "\n   register int data_out_int asm(\"20\");\n";
   outputFile << "   register int data_out_float asm(\"21\");\n";

   outputFile << "\n   /* Set Loop */\n";
   outputFile << "   unsigned int counter;\n";

   outputFile << "\n   /* Initialize Private Memory Region */\n";
   outputFile << "   int* memInt;\n";
//    outputFile << "   float* memFloat;\n";
   outputFile << "   int data_out_int_base;\n";
//    outputFile << "   int data_out_float_base;\n";

   //loop counters
   outputFile << "   register int cell_counter asm(\"23\");\t\t//inner loop counter\n";
   outputFile << "   register int block_counter asm(\"14\");\t\t//outer loop counter\n";

   outputFile << "\n";
   outputFile << "   memInt           = (int*)malloc(sizeof(int) * " << memSize << ");\n";
   outputFile << "   data_out_int     = (int)&(memInt[0]);\n";
//    outputFile << "   memFloat         = (float*)malloc(sizeof(float) * " << memSize << ");\n";
//    outputFile << "   data_out_float   = (int)&(memFloat[0]);\n";

   outputFile << "\n";
   outputFile << "   shared_memInt    = (int*)malloc(sizeof(int) * " << memSize << ");\n";
   outputFile << "   s_data_out_int   = (int)&(shared_memInt[0]);\n";
//    outputFile << "   shared_memFloat  = (float*)malloc(sizeof(float) * " << memSize << ");\n";
//    outputFile << "   s_data_out_float = (int)&(shared_memFloat[0]);\n";

   outputFile << "\n";
   outputFile << "   s_data_out_int_base = s_data_out_int;\n";
//    outputFile << "   s_data_out_float_base = s_data_out_float;\n";
   outputFile << "   data_out_int_base = data_out_int;\n";
//    outputFile << "   data_out_float_base = data_out_float;\n";
   outputFile << "\n";

   //thread spawns
   outputFile << "   //spawn threads\n";
   for(UINT_32 threadCounter = 1; threadCounter < numThreads; threadCounter++)
   {
      outputFile << "   sesc_spawn(threadFunc" << threadCounter << ", NULL, 0);";
      outputFile << "\n";
   }

   outputFile << std::endl;

}

/**
 * @ingroup Body
 *
 * @param outputFile 
 */
void Body::trailerGen(std::ofstream &outputFile)
{
   /* Variables */

   /* Processes */
   outputFile << "\n   return 0;\n}\n";
}


/**
 * @ingroup Body
 * 
 * @param threadID 
 * @param outputFile 
 */
void Body::funcHeaderGen(THREAD_ID threadID, std::ofstream &outputFile)
{
   /* Variables */
   UINT_32 memSize = MEM_REGION;

   /* Processes */
   outputFile << "//  This file was automatically generated.\n//  Do not edit this file.\n//\n//\n";
   outputFile << "/// @file param_prog" << "_" << threadID << ".h\n";
   outputFile << "/// @author IDEAL Lab, University of Florida\n";
   outputFile << "/// @author Clay Hughes, James Poe, and Tao Li\n";
   outputFile << "//\n// Copyright: See COPYING file that comes with this distribution\n//\n///////////////////////////////////////////////////////////////////////////////////\n";

   outputFile << "\n\nvoid threadFunc" << Instruction::IntToString(threadID) << "(void *ptr)\n";
   outputFile << "{\n";

   outputFile << "   register int r_out_t0 asm(\"8\");\n";
   outputFile << "   register int r_out_t1 asm(\"9\");\n";
   outputFile << "   register int r_out_t2 asm(\"10\");\n";
   outputFile << "   register int r_out_t3 asm(\"11\");\n";
   outputFile << "   register int r_out_t4 asm(\"12\");\n";
   outputFile << "   register int r_out_t5 asm(\"13\");\n";

   outputFile << "   register int r_out_s0 asm(\"16\");\n";
   outputFile << "   register int r_out_s1 asm(\"17\");\n";
   outputFile << "   register int r_out_s2 asm(\"18\");\n";
   outputFile << "   register int r_out_s3 asm(\"19\");\n";
   outputFile << "   register int r_out_s4 asm(\"20\");\n";
   outputFile << "   register int r_out_s5 asm(\"21\");\n";

   outputFile << "   register int r_out_f2 asm(\"$f2\");\n";
   outputFile << "   register int r_out_f4 asm(\"$f4\");\n";
   outputFile << "   register int r_out_f6 asm(\"$f6\");\n";
   outputFile << "   register int r_out_f8 asm(\"$f8\");\n";
   outputFile << "   register int r_out_f10 asm(\"$f10\");\n";
   outputFile << "   register int r_out_f12 asm(\"$f12\");\n";

   outputFile << "\n   register int data_out_int asm(\"20\");\n";
   outputFile << "   register int data_out_float asm(\"21\");\n";

   //loop counters
   outputFile << "   register int cell_counter asm(\"23\");\t\t//inner loop counter\n";
   outputFile << "   register int block_counter asm(\"14\");\t\t//outer loop counter\n";

   outputFile << "\n   /* Set Loop */\n";
   outputFile << "   unsigned int counter;\n";

   outputFile << "\n   /* Initialize Private Memory Region */\n";
   outputFile << "   int* memInt;\n";
//    outputFile << "   float* memFloat;\n";

   outputFile << "\n";
   outputFile << "   memInt         = (int*)malloc(sizeof(int) * " << memSize << ");\n";
   outputFile << "   data_out_int   = (int)&(memInt[0]);\n";
//    outputFile << "   memFloat       = (float*)malloc(sizeof(float) * " << memSize << ");\n";
//    outputFile << "   data_out_float = (int)&(memFloat[0]);\n";

   outputFile << "\n";
   outputFile << "   int data_out_int_base = data_out_int;\n";
//    outputFile << "   int data_out_float_base = data_out_float;\n";

   outputFile << std::endl;

}


/**
 * @ingroup Body
 * 
 * @param threadID 
 * @param outputFile 
 */
void Body::funcTrailerGen(THREAD_ID threadID, std::ofstream &outputFile)
{
   /* Variables */

   /* Processes */
   outputFile << "\n";
   outputFile << "}\n";
}
