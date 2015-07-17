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

// This program writes out an AMI_STREAM of random integers of
// user-specified length, and then, based on 7 partitioning elements
// chosen from that stream, partitions that stream into 8 buckets. Each
// of the buckets is implemented as an AMI stream and the program
// prints the size of each bucket at the end.

// The user needs to specify the length of the initial stream of
// integers and the size of the main memory that can be used.

#include <portability.h>




// Include the file that sets application configuration: It sets what
// kind of BTE (Block Transfer Engine) to use and where applicable,
// what should be the size of the logical block (the logical block size
// is a user specified multiple of the physical block size) for a
// stream and so on;
#include "app_config.h"

// Include the file that will allow us to use AMI_STREAMs.
#include <stream.h>

// Include timer that will allow us to time the program.
#include <cpu_timer.h>

// Include command line parsing functions
#include "getopts.h"

//set up command line options.
struct options opts[] = {
   {1, "length", "Number of integers to generate", "l", 1},
   {2, "mem", "Memory size (in bytes) to use", "m", 1},
   {0, NULL, NULL, NULL, 0} //getopts requires last option to be emtpy
};

//Tell user what program does and how to use it
//if they do not use proper options.
void print_usage(char * progname){
  printf("\nThis program writes out an AMI_STREAM of random integers of\n"
         "user-specified length, and then, based on 7 partitioning\n"
         "elements chosen from that stream, partitions that stream\n"
         "into 8 buckets. Each of the buckets is implemented as an\n"
         "AMI stream and the program prints the size of each bucket\n"
         "at the end. All created streams are deleted before exiting\n\n");
  getopts_usage(progname, opts);
  printf("\nSuffixes K, M, and G can be appended to the\n"
      "--length and --mem options to mean\n"
      "*1024, *1024*1024, and *2^30 respectively\n"
      "e.g., --length 10M --mem 32M creates roughly 10 million elements\n"
      "and can use a maximum of 32 MB of memory\n\n");
  printf("Sample usage:\n%s -l 50M -m 32M\n"
         "Writes 50 million random integers and partitions them using\n"
         "no more than 32 MB of memory\n\n", progname);
}

// Convert a string to a number
// Just like atoi or atol, but should also work for 64bit numbers
// Also supports KMG suffixes (e.g. 2K = 2*1024)
TPIE_OS_OFFSET ascii2offset(char *s){
  int i, len, digit;
  TPIE_OS_OFFSET multfactor, value;
  bool ok;
  
  i=0;
  len=strlen(s);
  value=0;
  
  if (len < 1){ return 0; }
  
  //look for KMG suffix
  switch(s[len-1]){
    case 'K':
    case 'k':
      multfactor = 1024;
      break;
    case 'M':
    case 'm':
      multfactor = 1024*1024;
      break;
    case 'G':
    case 'g':
      multfactor = 1024*1024*1024;
      break;
    default:
      multfactor = 1;
      break;
  }
  
  //convert string to decimal
  ok=true;
  do {
    digit=s[i]-'0';
    if((digit< 0) || (digit > 9)){ ok = false;} //stop on non-digit
    else{value = 10*value+digit;}
    i++;
  } while((i<len) && ok);

  return value*multfactor;
}

void get_app_info(int argc, char** argv, 
                  TPIE_OS_OFFSET& len, TPIE_OS_OFFSET& mem){

  int optidx, opts_set=0;
  char* optarg;

  if(argc<5){
    //not enough options specified
    print_usage(argv[0]);
    exit(1);
  }
  
  while(optidx=getopts(argc, argv, opts, &optarg)){
    if(optidx==-1){
      printf("Could not allocate space for arguments. Exiting...\n");
      exit(1);
    }
    switch(optidx){
      case 1:
        len=ascii2offset(optarg);
        opts_set=opts_set | 1;
        break;
      case 2:
        mem=ascii2offset(optarg);
        opts_set=opts_set | 2;
        break;
      default:
        printf("Unhandled option - %d\n",optidx);
        break;
    }//end switch
  }//end while 
  if(opts_set != 3){
    printf("Both length and memory must be specified\n\n");
    print_usage(argv[0]);
    exit(1);
  }
}

// The user needs to specify the length of the initial stream of
// integers and the size of the main memory that can be used.
int main(int argc, char *argv[]) { 
  TPIE_OS_OFFSET Gen_Stream_Length;
  TPIE_OS_OFFSET test_mm_size;
 
  //get length, mem size from command line
  get_app_info(argc, argv, Gen_Stream_Length, test_mm_size);

  std::cout << "Writing " << Gen_Stream_Length << " random integers\n"
       << "using a maximum " << test_mm_size << " bytes of memory\n"<<endl;
  
  //Tell the memory manager to abort if the allocated 
  //internal memory exceeds the specified amount
  MM_manager.enforce_memory_limit();

   //Set the size of memory the application is allowed to use
   MM_manager.set_memory_limit(test_mm_size);
   
   //the source stream of ints
   AMI_STREAM<int> source;

   //the 8 bucket streams of ints
   AMI_STREAM<int> buckets[8];
   
   // ************************************************************
   // Generate the stream of randon integers
   AMI_err ae;
   int src_int;
   TPIE_OS_OFFSET i;
   std::cout << "Writing random stream..."<<endl;
   for (i = 0; i < Gen_Stream_Length; i++) {
      
      // Generate a random int.
      src_int = TPIE_OS_RANDOM();
      
      // Write out the integer into the AMI_STREAM source using 
      // the AMI_STREAM member function write_item()
      if ((ae = source.write_item(src_int)) != AMI_ERROR_NO_ERROR) {
	cout << "AMI_ERROR " << ae << " during source.write_item()" << std::endl;
         exit(1);
      }      
   }

   // Print stream length
   std::cout << "source stream is of length " << source.stream_len() << std::endl;
   
   // ************************************************************
   // Pick the first 7 integers in source stream as partitioning elements
   // (pivots)
   
   // Seek to the beginning of the AMI_STREAM source.
   if ((ae = source.seek(0))!= AMI_ERROR_NO_ERROR) {
     std::cout << "AMI_ERROR " << ae << " during source.seek()" << std::endl;
      exit(1);
   }
   
   // Read first 7 integers and fill in the partitioning array.
   int partitioning[8];
   int *read_ptr;
   
   for (i = 0; i < 7; i++) {
      
      // Obtain a pointer to the next integer in AMI_STREAM source
      // using the member function read_item().
      if ((ae = source.read_item(&read_ptr)) != AMI_ERROR_NO_ERROR) {
	cout << "AMI_ERROR " << ae << " during source.read_item()" << std::endl;
         exit(1);
      }
      
      // Copy the current source integer into the partitioning element array.
      partitioning[i]= *read_ptr;
   }
   std::cout << "Loaded partitioning array" << std::endl;

   // ************************************************************
   // Sort partitioning array
   
   std::sort(&partitioning[0],&partitioning[6]);
   std::cout << "Sorted partitioning array" << std::endl;
   partitioning[7] = INT_MAX;

   // ************************************************************
   // PARTITION INTS OF source INTO THE buckets USING partitioning ELEMENTS
   
   // Binary search variables.
   int u, v, l, j;
   
   std::cout << "Partitioning elements into 8 buckets..." <<endl;
   // Start the timer.
   cpu_timer timer;
   timer.start();
   
   // Seek to the beginning of the AMI_STREAM source.
   if ((ae = source.seek(0))!= AMI_ERROR_NO_ERROR) {
     std::cout << "AMI_ERROR " << ae << " during source.seek()" << std::endl;
      exit(1);
   }
   
   // Scan source stream distributing the integers in the approriate
   // buckets
   for (i = 0; i < Gen_Stream_Length; i++) {
      
      // Obtain a pointer to the next integer in AMI_STREAM source
      // using the member function read_item()
      if ((ae =   source.read_item(&read_ptr)) != AMI_ERROR_NO_ERROR) {
	cout << "AMI_ERROR " << ae << " during source.read_item()" << std::endl;
         exit(1);
      }
      v = *read_ptr;

      // Using a binary search, find the stream index l to which v 
      // should be assigned.
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

      // Write out the int into the AMI_STREAM buckets[l] using 
      // the AMI_STREAM member function write_item().
      if ((ae = buckets[l].write_item(v)) != AMI_ERROR_NO_ERROR) {
         std::cout << "AMI_ERROR " << ae << " during buckets[" << l 
	      << "].write_item()" << std::endl;
         exit(1);
      }
   }
   
   // Stop the timer.
   timer.stop();
   std::cout << "Time taken to partition is " << timer.wall_time() 
	<< " seconds" << std::endl;
   
   // Delete the file corresponding to the source stream when source
   // stream gets destructed (this is the default, so this call is not
   // needed).
   source.persist(PERSIST_DELETE);
   
   // Print the lengths of the bucket streams.
   for (i = 0; i < 8; i++) {
      std::cout << "Length of bucket " << i << " is " 
	   << buckets[i].stream_len() << std::endl;
   }

   std::cout << "Program ran successfully" << std::endl;
   return 0;
}
