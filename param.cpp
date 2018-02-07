/**
 * @file
 * @author James Poe     <James Poe@samara>, (C) 2008, 2009
 * @author Clay Hughes   <Clay Hughes@fraidy2-uf>, (C) 2008, 2009
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 * Moo cows rule.
 *
 * @brief Main file for TransPlant
*//////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <errno.h>
#include <list>

#include "Config.h"
#include "ConfigValidator.h"
#include "Skeleton.h"
#include "ConstructSkeleton.h"
#include "AssimilateSkeleton.h"
#include "Cell.h"
#include "Skin.h"
#include "Body.h"
#include "Compressor.h"

using std::cout;

int main(int argc, char *argv[])
{
   Config *config;
   Skeleton skeleton;
   BOOL assimilateMode = 0;
   BOOL compress = 0;

   UINT_32  numLoops = 1;                                                                          //number if iterations per thread
   BOOL     resetMemPerCell = 0;                                                                   //reset mem per cell?
   BOOL     barrier_per_thread = 0;                                                                //should there be a barrier at the start of each thread?

   if(argc < 2)
   {
      std::cerr << "Usage: ./param [options] descriptor\n";
      std::cerr << "Options:\n";
      std::cerr << "\t[-a bool]\t\t Enable assimilation\n";
      std::cerr << "\t[-l num_loops]\t\t Number of loops in the main program\n";
      std::cerr << "\t[-m bool]\t\t Reset memory in each cell\n";
      std::cerr << "\t[-b bool]\t\t Enable barrier sync per thread\n";
      std::cerr << std::endl;

      exit(0);
   }
   else
   {
      int c;
      opterr = 0;
      while (( c = getopt( argc, argv, "al:m:b:" ) ) != -1 )
      {
         switch ( c )
         {
            case 'a':
               assimilateMode = 1;
               break;
            case 'l':
               numLoops = atoi(optarg);
               break;
            case 'm':
               resetMemPerCell = atoi(optarg);
               break;
            case 'b':
               barrier_per_thread = atoi(optarg);
               break;
            case '?':
               return 1;
               break;
         }
      }

      std::ifstream inputFile(argv[optind]);
      if(!inputFile)                                                //check to be sure file is open
      {
         std::cerr << "Error opening file.\n";
         exit(0);
      }

      std::cout << "\n\n\t\t\tWelcome to TransPlant\n";

      if( assimilateMode )
      {
         std::cout << "Assimilate Mode Enabled...\n";
      }
   }

   if( assimilateMode )
   {
      config = new Config();
      AssimilateSkeleton skelImporter ( argv[optind] );
      skeleton = skelImporter.loadSkeleton();
      config->add<unsigned int>( "Global" , "numThreads", skeleton.getNumThreads() );
      config->add<int>( "Global" , "numBarriers", 0 );
   }
   else
   {
      //read the file into the configuration tool
      config = new Config(argv[optind]);

      if(!ConfigValidator::validate(config))
      {
         cerr << "Failed Validation" << endl;
         return 1;
      }

      ConstructSkeleton skelGenerator(config);

      skeleton = skelGenerator.createSkeleton();
   }

//    #if defined(VERBOSE)
//    unsigned int numThreads = skeleton.getNumThreads();
//    for ( unsigned int i = 0; i < numThreads; i++)
//    {
//       cout << "\nTHREAD: " << i << endl;
//       THREAD_CELL_DEQUEP tdeq = skeleton.getThread( i );
//       for ( unsigned int x = 0; x < tdeq->size() ; x = x + 1)
//       {
//          cout << endl << x;
//          cout << *(tdeq->at (x )) << endl;
//       }
//    }
//    #endif

   //compression
   if(compress == 1)
   {
      std::cout << "Compression Enabled...\n";
      for(UINT_32 threadID = 0; threadID < skeleton.getNumThreads(); threadID++)
      {
         Compressor compress(skeleton.getThread(threadID));
         compress.reduceSequential();
         compress.check();
         compress.compression();
         compress.generateNewCellList();
         compress.printCompleteRuleList();

         skeleton.setThread(compress.get_reduced_cellList(), threadID);
      }
   }

   //update configuration paramters
   config->add<bool>( "Global", "barrierPerThread", barrier_per_thread);
   config->add<string>("Global", "fileName", string(argv[optind]).substr(string(argv[optind]).find_last_of("/") + 1));

   if(numLoops != 1 || assimilateMode == 1)
      config->add<unsigned int>( "Global" , "numLoops", numLoops);
   if(resetMemPerCell != 0 || assimilateMode == 1)
      config->add<bool>( "Global", "resetPerCell", resetMemPerCell);

   //convert cells to instructions
   Skin skin(skeleton);
   skin.updateConfig(config);
   skin.spinalColumn();
   skin.insertVertebrae();
   if(config->read<int>("Global", "numBarriers") > 0)
      skin.synchronize();

   Body body(skin);
   body.writeProgram();

   //clean up and exit
   delete config;
   std::cout << "All finished! You can find the new source code in:  ../param/output" << "\nHappy testing!\n";

   return 0;
}


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

