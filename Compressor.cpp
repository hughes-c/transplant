/**
 * @file
 * @author  Clay Hughes   <>, (C) 2008, 2009
 * @date    11/11/08
 * @brief   This is the implementation of the Compressor object.
 *
 * @section LICENSE
 * Copyright: See COPYING file that comes with this distribution
 *
 * @section DESCRIPTION
 * C++ Implementation: Compressor
 */
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#include "Compressor.h"

int num_rules = 0;      // number of rules in the grammar
int num_symbols = 0;    // number of symbols in the grammar
int numbers = 1;
int delimiter = -1;
int K = 1;              // minimum number of times a digram must occur to form rule, decreased by one (e.g. if K is 1, 2 occurrences are required to form rule) NOTE must be '1'

char *delimiter_string = 0;

#include "SequiterClasses.h"

Compressor::Compressor(deque < Cell * > *cellIn)
{
   cellList = cellIn;
   maxRuleLength = 2;
   wasLoop = 0;

   reduced_cellList = new std::deque < Cell * >;
}

Compressor::~Compressor()
{
   delete reduced_cellList;
}

deque< Cell * >  *Compressor::get_reduced_cellList()
{
  return reduced_cellList;
}

void Compressor::check()
{
   /* Variables */
   std::map <UINT_32, std::deque< UINT_32 > >::iterator matchIterator;

   /* Processes */
   for(UINT_32 outer = 0; outer < cellList->size(); outer++)
   {
      for(UINT_32 inner = 0; inner < cellList->size(); inner++)
      {
         //this is where the TOL comes in
         if(*(cellList->at(outer)) == *(cellList->at(inner)))
         {
            #if defined(VERBOSE)
            std::cout << "\n" << outer << " equal to " << inner << "\n";
            #endif

            cellMatches[outer].push_back(inner);
         }
      }//end inner
   }//end outer

   //reset cell alias
   for(UINT_32 cellID = 0; cellID < cellList->size(); cellID++)
   {
      cellAliases[cellID] = -1;
   }

   //assign new alias
   for(std::map<UINT_32, std::deque<UINT_32> >::iterator narp = cellMatches.begin(); narp != cellMatches.end(); narp++)
   {
      for(std::deque<UINT_32>::iterator yar = narp->second.begin(); yar != narp->second.end(); yar++)
      {
         if(cellAliases[*yar] == -1)
            cellAliases[*yar] = narp->first; 
      }
   }

   #if defined(VERBOSE)
   for(std::map<UINT_32, std::deque<UINT_32> >::iterator narp = cellMatches.begin(); narp != cellMatches.end(); narp++)
   {
      std::cout << "\n" << narp->first << ":  ";
      for(std::deque<UINT_32>::iterator yar = narp->second.begin(); yar != narp->second.end(); yar++)
      {
         std::cout << *yar << " ";
      }
   }

   for(std::map <UINT_32, INT_32 >::iterator narp = cellAliases.begin(); narp != cellAliases.end(); narp++)
   {
      std::cout << "\n" << narp->first << " : " << narp->second;
   }
   #endif

}

void Compressor::compression()
{
   /* Processes */
   for(std::map <UINT_32, INT_32 >::iterator narp = cellAliases.begin(); narp != cellAliases.end(); narp++)
   {
      if(reductionString.size() < reductionString.max_size() + 2)
      {
         reductionString = reductionString + Instruction::IntToString(narp->second) + ":";
      }
      else
      {
         std::cerr << "\nError:  Maximum string size exceeded." << std::endl;
         exit(1);
      }
   }

   #if defined(DEBUG)
   std::cout << "\n\nReduction String:  " << reductionString;
   UINT_32 shutTheHellUp = 0;
   for(string::iterator boo = reductionString.begin(); boo != reductionString.end(); boo++) if(*boo == ':') ++shutTheHellUp;
   std::cout << " -- " << shutTheHellUp << "\n";
   #endif

   buildRules();
}

void Compressor::buildRules()
{
   /* Variables */
   std::ofstream *rule_S = 0;

   S = new rules;
   INT_32 symbolNum = 0;
   min_terminal = max_terminal = symbolNum;
   std::basic_string<char>::iterator stringIterator = reductionString.begin();

   /* Processes */
   //convert the string to the list(s) of grammars
   do
   {
      string tempString = "";
      while(*stringIterator != ':' && stringIterator != reductionString.end())
      {
         tempString = tempString + *stringIterator;
         ++stringIterator;
      }
      if(stringIterator != reductionString.end())
         ++stringIterator;

      symbolNum = atoi(tempString.c_str());

      if(symbolNum < min_terminal)
         min_terminal = symbolNum;
      else if(symbolNum > max_terminal)
         max_terminal = symbolNum;

      //append character to end of rule S, and enforce constraints
      S->last()->insert_after(new symbols(symbolNum));
      S->last()->prev()->check();

   }while(stringIterator != reductionString.end());

   //allocate memory for rule list
   R1 = (rules **) malloc(sizeof(rules *) * num_rules);
   memset(R1, 0, sizeof(rules *) * num_rules);
   R1[0] = S;
   Ri = 1;

   for(INT_32 i = 0; i < Ri; i ++)
   {
      for(symbols *p = R1[i]->first(); !p->is_guard(); p = p->next())
      {
         if (p->nt() && R1[p->rule()->index()] != p->rule())
         {
            p->rule()->index(Ri);
            R1[Ri ++] = p->rule();
         }
      }
   }//end outer for
}

void Compressor::generateNewCellList()
{
   /* Variables */
   UINT_32 root = 0;
   UINT_32 elementCount;
   INT_32  elementLookup;

   /* Processes */
   //print list
   std::cout << "\nReduced List:\n";
   for(symbols *p = R1[root]->first(); p->is_guard() == 0; p = p->next())
   {
      if(p->nt())
         std::cout << p->rule()->index() << ' ';
      else
         std::cout << *p << ' ';
   }
   std::cout << std::endl;

   //go through the root rule list and look for succuessive matches
   for(symbols *p = R1[root]->first(); p->is_guard() != 1; )
   {
      symbols *n;
      elementCount = 1;

      //look through the symbol list until the successive element doesn't match
      for(n = p->next(); n->is_guard() != 1; )
      {
         //non-terminals
         if(p->nt() == 0)
         {
            if(n->nt() == 0 && n->value() == p->value())                                     //NOTE Order of test matters here
            {
               elementCount = elementCount + 1;

               #if defined(VERBOSE)
               std::cout << "---" << *n << " = " << *p <<  "\t" << elementCount << "\n" << std::flush;
               #endif
            }
            else
            {
               break;
            }
         }
         //terminals
         else
         {
            if(n->nt() == 1 && n->rule()->index() == p->rule()->index())                     //NOTE Order of test matters here
            {
               elementCount = elementCount + 1;

               #if defined(VERBOSE)
               std::cout << "---" << n->rule()->index() << " = " << p->rule()->index() << "\t" << elementCount << "\n" << std::flush;
               #endif
            }
            else
            {
               break;
            }
         }

         n = n->next();
      }

      //if there is a loop , insert loop start cell
      if(elementCount > 1)
      {
         Cell *temp = new Cell();
         temp->setCellType(LoopStart);
         temp->setLoopCount(elementCount);
         reduced_cellList->push_back(temp);
      }

      //add the cell
      if(p->nt() == 0)
      {
// std::cout << "\n+++p:  " << *p;
         reduced_cellList->push_back(cellList->at(p->value()));
      }
      else
      {
         //this is a rule so we need to iterate until we find the terminals
         cellIDs = new std::deque < UINT_32 >;
         rootIndex = p->rule()->index();

         if(checkRules(p) != 0)
         {
// std::cout << "\n***(" << checkRules(p) << ")p:  " << *p;

            UINT_32 newElementCount = elementCount;
            newElementCount = newElementCount * buildSuperCells(rootIndex);

            if(elementCount > 1)
               reduced_cellList->back()->setLoopCount(newElementCount);
         }
         else
         {
// std::cout << "\n---p:  " << *p;
            addCell(p);
         }

         //add the cell IDs from the previous function to the reduced cell list
         for(std::deque < UINT_32 >::iterator boo = cellIDs->begin(); boo != cellIDs->end(); boo++)
         {
            reduced_cellList->push_back(cellList->at(*boo));
         }

         delete cellIDs;
      }

      //if there is a loop, insert loop end cell
      if(elementCount > 1)
      {
         Cell *temp = new Cell();
         temp->setCellType(LoopEnd);
         reduced_cellList->push_back(temp);
      }

      //set p to n so that we continue where we left off
      p = n;

   }///end for
}

void Compressor::addCell(symbols *symbolIn)
{
   /* Variables */
   UINT_32 index = symbolIn->rule()->index();
   /* Processes */
   for(symbols *symTest = R1[index]->first(); symTest->is_guard() != 1; symTest = symTest->next())
   {
      //if this is a terminal add it to the list, otherwise expand
      if(symTest->nt() == 0)
      {
// std::cout << "\n--addCell " << *symTest;
         cellIDs->push_back(symTest->value());
      }
      else
      {
         addCell(symTest);
      }
   }
}

UINT_32 Compressor::checkRules(symbols *symbolIn)
{
   /* Variables */
   BOOL same = 0;
   UINT_32 index = symbolIn->rule()->index();
// std::cout << "\n--checkRules " << *symbolIn << ":  " << std::flush;
   /* Processes */
   for(symbols *symTest = R1[index]->first(); symTest->is_guard() != 1; symTest = symTest->next())
   {
// std::cout << *symTest << " " << std::flush;
      //if this is a terminal and ALL of its successors are terminals, we're finished
      if(symTest->nt() == 0)
      {
         for(symbols *termTest = symTest->next(); termTest->is_guard() != 1; termTest = termTest->next())
         {
// std::cout << *termTest << " " << std::flush;
            //if this is a non-terminal then this is not the end and we return 0
            if(termTest->nt() == 1)
               return 0;
         }

         return 3;
      }
      else
      {
         //iterate through the rule, if there are subsequent rules then check to see if they are the same rule
         for(symbols *termTest = symTest->next(); termTest->is_guard() != 1; termTest = termTest->next())
         {
// std::cout << *termTest << " " << std::flush;
            if(termTest->nt() == 1)
            {
               if(termTest->rule()->index() != symTest->rule()->index())
               {
                  same = 0;
                  break;
               }
               else
               {
                  same = 1;
               }
            }
            else
            {
               same = 0;
               break;
            }
         }

         if(same == 1)
            return (checkRules(symTest));
         else
            return 0;
      }
   }
}

UINT_32 Compressor::buildSuperCells(UINT_32 index)
{
   /* Variables */
   UINT_32 multiplier = 1;
   symbols *symTest = R1[index]->first();
std::cout << "\nSuper-- " << *symTest << std::flush;
   /* Processes */
   //if this is a terminal and ALL of its successors are terminals, we're finished
   if(symTest->nt() == 0)
   {
      for(symbols *termTest = symTest; termTest->is_guard() != 1; termTest = termTest->next())
      {
         cellIDs->push_back(termTest->value());
      }
   }
   else
   {
      //we know that every non-terminal in this rule is equal so we just need to get to the last one
      for(symbols *termTest = symTest->next(); termTest->is_guard() != 1; termTest = termTest->next())
      {
         if(termTest->rule()->index() == symTest->rule()->index())
         {
            multiplier = multiplier + 1;
         }
      }

      multiplier = multiplier * buildSuperCells(symTest->rule()->index());
   }

   return multiplier;
}

void Compressor::printRule(UINT_32 index)
{
   /* Processes */
   std::cout << "\nRule " << index << "\n";

   std::cout << index << " -> ";
   for(symbols *p = R1[index]->first(); p->is_guard() == 0; p = p->next())
   {
      if(p->nt() == 1)
         std::cout << p->rule()->index() << ' ';
      else
         std::cout << *p << ' ';
   }

   std::cout << std::endl;
}

void Compressor::printCompleteRuleList()
{
   std::cout << "\nComplete Rule List:\n";

   for(INT_32 i = 0; i < Ri; i ++)
   {
      std::cout << i << " -> ";
      for(symbols *p = R1[i]->first(); !p->is_guard(); p = p->next())
      {
         if(p->nt() == 1)
            std::cout << p->rule()->index() << ' ';
         else
            std::cout << *p << ' ';
      }

      std::cout << std::endl;
   }
}

//!----------------------------------------------
void Compressor::reduceSequential(void)
{
   /* Variables */
   BOOL isFirst = 0;
   BOOL track = 0;
   Cell *previousSeqCell;
   Cell *previousTransCell;

   float totalInstructionLength = 0.0;
   float averageInstructionLength = 0.0;

   cellIDs = new std::deque < UINT_32 >();

   UINT_32 depth = 0;
   UINT_32 previousDepth = 0;

   /* Processes*/
   for(UINT_32 cellID = 0; cellID < cellList->size(); cellID++)
   {
      //cell type (Tx - Sq)
      if(cellList->at(cellID)->getCellType() == Transactional)
      {
         if(track == 0)
         {
            track = 1;
         }
         else if(*previousTransCell == *cellList->at(cellID))
         {
            if(isFirst == 0)
            {
               isFirst = 1;
               cellIDs->push_back(cellID - 3);
            }

            cellIDs->push_back(cellID - 1);
         }
         else
         {
            isFirst = 0;

            if(cellIDs->size() > 0)
            {
               for(std::deque<UINT_32>::iterator boo = cellIDs->begin(); boo != cellIDs->end(); boo++)
               {
                  totalInstructionLength = totalInstructionLength + cellList->at(*boo)->getNumInstructions();
               }

               totalInstructionLength = 0.0;
               averageInstructionLength = totalInstructionLength / cellIDs->size();

               for(UINT_32 counter = previousDepth; previousDepth < depth; previousDepth++)
               {
                  reduced_cellList->push_back(cellList->at(previousDepth));
               }

               previousDepth = depth;
               depth = 0;

               reduced_cellList->push_back(generateSeqCell(roundFloat(averageInstructionLength)));
               reduced_cellList->push_back(previousTransCell);

               delete cellIDs;
               cellIDs = new std::deque < UINT_32 >();
            }
         }

         previousTransCell = cellList->at(cellID);
      }

      depth = depth + 1;
   }

for(std::deque < Cell * >::iterator boo= reduced_cellList->begin(); boo != reduced_cellList->end(); boo++)
{
   std::cout << (*boo)->getNumInstructions() << "\n";
}

}

Cell* Compressor::generateSeqCell(UINT_32 numInstructions)
{
   Cell *boo = new Cell();

   return boo;
}


