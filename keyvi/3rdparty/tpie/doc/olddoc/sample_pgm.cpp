// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#include <iostream.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h> //for INT_MAX
 
//Include the file that sets application configuration: It sets what
//kind of BTE (Block Transfer Engine) to use and where applicable,
//what should be the size of the logical block (the logical block size
//is a user specified multiple of the physical block size) for a
//stream and so on;
#include "app_config.h"

//Include the file that will allow us to use AMI_STREAMs.
#include <ami.h>

//include "wall clock" timer that will allow us to time
#include <wall_timer.h>

//Include TPIE's internal memory sorting routines.
#include <quicksort.h>

//This program writes out an AMI_STREAM of random integers of
//user-specified length, and then, based on 7 partitioning elements
//chosen from that stream, partitions that stream into 8 buckets. Each
//of the buckets is implemented as an AMI stream and the program
//prints the size of each bucket at the end.

//The user needs to specify the length of the initial stream of
//integers and the size of the main memory that can be used.


void
main(int argc, char *argv[]) {
   
   //parse arguments
   if (argc < 3) {
      cout << "Input the number of integers to be generated\n";
      cout << " and the size of memory that can be used\n";
      exit(1);
   }
   int Gen_Stream_Length = atoi(argv[1]);
   long test_mm_size = atol(argv[2]);
   
   //Set the size of memory the application is allowed to use
   MM_manager.set_memory_limit(test_mm_size);
   
   //the source stream of ints
   AMI_STREAM<int> source;

   //the 8 bucket streams of ints
   AMI_STREAM<int> buckets[8];
   
   
   //************************************************************
   //generate the stream of randon integers
   AMI_err ae;
   int src_int;
   for (int i = 0; i < Gen_Stream_Length; i++) {
      
      //generate a random int
      src_int = random();
      
      //Now write out the integer into the AMI_STREAM source using 
      //the AMI_STREAM member function write_item()
      if ((ae = source.write_item(src_int)) != AMI_ERROR_NO_ERROR) {
         cout << "AMI_ERROR " << ae << " during source.write_item()\n";
         exit(1);
      }
      
   }
   //print stream length
   cout << "source stream is of length " << source.stream_len() << endl;
   
   //************************************************************
   //pick the first 7 integers in source stream as partitioning elements
   //(pivots)
   
   //Seek to the beginning of the AMI_STREAM source.
   if ((ae = source.seek(0))!= AMI_ERROR_NO_ERROR) {
      cout << "AMI_ERROR " << ae << " during source.seek()\n";
      exit(1);
   }
   
   //read first 7 integers and fill in the partitioning array
   int partitioning[8];
   int *read_ptr;
   
   for (int i = 0; i < 7; i++) {
      
      //Obtain a pointer to the next integer in AMI_STREAM source
      //using the member function read_item()
      if ((ae = source.read_item(&read_ptr)) != AMI_ERROR_NO_ERROR) {
         cout << "AMI_ERROR " << ae << " during source.read_item()\n";
         exit(1);
      }
      
      //Copy the current source integer into the partitioning element array.
      partitioning[i]= *read_ptr;
   }
   cout << "Loaded partitioning array\n";

   //************************************************************
   //sort partitioning array
   
   quick_sort_op((int *)partitioning,7);
   cout << "sorted partitioning array\n";
   partitioning[7] = INT_MAX;

   //************************************************************
   //PARTITION INTS OF source INTO THE buckets USING partitioning ELEMENTS
   
   struct timeval tp1, tp2;

   //binary search variables.
   int u,v,l,j;
   
   //start timer
   wall_timer wt;
   wt.start();
   
   //seek to the beginning of the AMI_STREAM source.
   if ((ae = source.seek(0))!= AMI_ERROR_NO_ERROR) {
      cout << "AMI_ERROR " << ae << " during source.seek()\n";
      exit(1);
   }
   
   //scan source stream distributing the integers in the approriate
   //buckets
   for (int i = 0; i < Gen_Stream_Length; i++) {
      
      //Obtain a pointer to the next integer in AMI_STREAM source
      //using the member function read_item()
      if ((ae =   source.read_item(&read_ptr)) != AMI_ERROR_NO_ERROR) {
         cout << "AMI_ERROR " << ae << " during source.read_item()\n";
         exit(1);
      }
      v = *read_ptr;

      // using a binary search, find the stream index l to which v 
      // should be assigned
      l = 0;
      u = 7;
      while (u >= l) {
         j = (l+u)>>1; 
         if (v < partitioning[j]) {
            u = j-1;
         } else {
            l = j+1;
         }
      }

      // now write out the int into the AMI_STREAM buckets[l] using 
      // the AMI_STREAM member function write_item().
      if ((ae = buckets[l].write_item(v)) != AMI_ERROR_NO_ERROR) {
         cout << "AMI_ERROR " << ae << " during buckets[" << l 
                << "].write_item()\n";
         exit(1);
      }
   }
   
   //stop timer
   wt.stop();
   cout << "Time taken to partition is " << wt.seconds() << " seconds" << endl;
   
   //delete the file corresponding to the source stream when source
   //stream gets destructed (this is the default, so this call is not
   //needed)
   source.persist(PERSIST_DELETE);
   
   //let the file corresponding to buckets[i] persist on disk when the
   //buckets[i] stream gets destructed
   for (int i = 0; i < 8; i++) {
      buckets[i].persist(PERSIST_PERSISTENT);
      cout << "Length of bucket " << i << " is " 
             << buckets[i].stream_len() << endl;
   }  
}
