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

#include "app_config.h"
#include <tpie/tpie.h>
#include <tpie/portability.h>
#include "test_portability.h"
#include <tpie/tpie_log.h>
#include <tpie/stream.h>
#include <tpie/sort.h>
#include <tpie/err.h>
#include "getopts.h"
#include <algorithm>    //STL internal sort
#include <cstdlib>     //C internal sort
#include <tpie/cpu_timer.h>

#include <tpie/progress_indicator_arrow.h>

using namespace tpie;

enum app_options { 
  APP_OPTION_PATH = 1,
  APP_OPTION_NUM_ITEMS,
  APP_OPTION_FILE_SIZE,
  APP_OPTION_MEM_SIZE,
  APP_OPTION_EMPTY,
};

enum test_type {
  APP_TEST_OBJ_OP = 1,
  APP_TEST_PTR_OP,
  APP_TEST_OBJ_CMPOBJ,
  APP_TEST_PTR_CMPOBJ,
  APP_TEST_KOBJ
};

const unsigned int APP_OPTION_NUM_OPTIONS=5; 
const stream_offset_type APP_MEG = 1024*1024;
const stream_offset_type APP_GIG = 1024*1024*1024;
const stream_offset_type APP_DEFAULT_N_ITEMS = 10000;
const stream_offset_type APP_DEFAULT_MEM_SIZE = 128*1024*1024;
const int  APP_ITEM_SIZE = 1;

const char* APP_FILE_BASE =  "TPIE_Test";

// Basic parameters for the test
// better than having several global variables
typedef struct app_info{
    const char * path;
    int item_size;
    TPIE_OS_OFFSET num_items;
    TPIE_OS_SIZE_T mem_size;
} appInfo;

// ********************************************************** 
// class SortItem
// **********************************************************

// A class that contains a simple fixed-size array of chars 
// The array can also be used as a counter in base-26 arithmetic
class SortItem{
  public:
    SortItem(long key_=0);
    ~SortItem(){};
    long key;        //key to sort on
    bool operator<(const SortItem& rhs) const {
      return (this->key<rhs.key);
    }
  private:
    char item[APP_ITEM_SIZE];  //extra space to make bigger items
};

// Constructor
// Initialize item 'aaaaaaa...'
SortItem::SortItem(long key_): key(key_) {
  int i;
  for(i=0; i<APP_ITEM_SIZE-1; i++) item[i]='a';
  //null-terminate so we can print array as string
  item[APP_ITEM_SIZE-1]='\0';
}

// ********************************************************** 
// End Class SortItem
// ********************************************************** 

// Comparison class for STL/TPIE sorting
class SortCompare{
   public:
     SortCompare(){};
     //For STL sort
     inline bool operator()(const SortItem &left, const SortItem &right) const{
       return left.key<right.key;
     }
     //For TPIE sort
     inline int compare(const SortItem &left, const SortItem &right){
       if(left.key<right.key){return -1;}
       else if (left.key>right.key){return 1;}
       else return 0;
     }
};

// class for extracting, comparing keys for key sorting
class KeySortCompare{
  public:
    KeySortCompare(){};
    
    //Extract key from sortItem and store in given key reference
    inline void copy(long* key, const SortItem& item){
      *key=item.key;
    }
    //compare sortItem keys (longs)
    inline int compare(const long& left, const long& right){
      if(left<right){ return -1; }
      else if(left>right) {return 1; }
      else return 0;
    }
};

int qcomp(const void* left, const void* right){
    return static_cast<int>((static_cast<const SortItem*>(left)->key) -
			    (static_cast<const SortItem*>(right)->key));
}

// Just like atoi or atol, but should also work for 64bit numbers
// Also supports KMG suffixes (e.g. 2K = 2*1024)
TPIE_OS_OFFSET ascii2longlong(char *s){

  TPIE_OS_SIZE_T i, len, multfactor;
  TPIE_OS_SSIZE_T digit;
  TPIE_OS_OFFSET val;
  bool ok, neg;
  
  len = strlen(s);
  val = 0;
  i=0;
  ok=true;
  neg=false;
  multfactor=1;

  if(s[0]=='-'){ 
    neg = true;
    i++;
  }
  
  switch(s[len-1]){
    case 'K':
    case 'k':
      multfactor = 1024;
      break;
    case 'M':
    case 'm':
      multfactor = APP_MEG;
      break;
    case 'G':
    case 'g':
      multfactor = APP_GIG;
      break;
    default:
      break;
  }

  if(i<len){
    do {
      digit = s[i]-'0';
      if((digit< 0) || (digit > 9)){
        ok = false;
      }
      else{
        val = 10*val+digit;
      }
      i++;
    } while((i<len) && ok);
  }
 
  val *= multfactor;
  if(neg){ val = 0-val; }
  return val;
}

// init_opts sets up the options structures for use in getopts
void init_opts(struct options* & opts, int /* argc */, char** /* argv */){
  
  opts = new struct options[APP_OPTION_NUM_OPTIONS];
    
  int i=0;
  opts[i].number=APP_OPTION_PATH;
  opts[i].name = "path-name";
  opts[i].shortName = "p";
  //this option takes the requested path name as an argument
  opts[i].args = 1;
  opts[i].description = "Path for temp files";
  
  i++; 
  opts[i].number=APP_OPTION_NUM_ITEMS;
  opts[i].name = "numitems";
  opts[i].shortName = "n";
  //this option takes the requested item-size as an argument
  opts[i].args = 1;
  opts[i].description = "Number of items to write";

  i++;
  opts[i].number=APP_OPTION_FILE_SIZE;
  opts[i].name = "size";
  opts[i].shortName = "s";
  opts[i].args = 1;
  opts[i].description = "Create a file of size <args>";

  i++;
  opts[i].number=APP_OPTION_MEM_SIZE;
  opts[i].name = "memsize";
  opts[i].shortName = "m";
  opts[i].args = 1;
  opts[i].description = "Use no more memory than size <args>";

  // get opts looks for a structure with an empty description
  // not having one could lead to segfaults
  // is there a way for getopts to count options?
  i++;
  opts[i].number = APP_OPTION_EMPTY;
  opts[i].name = "";
  opts[i].shortName = "";
  opts[i].args=0;
  opts[i].description = 0;
}

// Print a warning when a user specifies multiple conflicting options. 
inline void print_warning(){
  static bool done=false;
  if(!done){
    std::cerr << "Warning: Only one of --numitems, or --size can be specified.\n"
         << "Ignoring extra options\n" << std::endl;
    done=true;
  }
}

// Set up application parameters based on command line args
// Displays help if no parameters are specified
void get_app_info(int argc, char** argv, appInfo & Info){

  struct options* opts;
  char* optarg;
  int optidx, nopts;
  bool optset=false;
  TPIE_OS_OFFSET tmp;

  init_opts(opts, argc, argv); 

  // get the dir
  const char* base_dir = getenv("AMI_SINGLE_DEVICE");
  if (base_dir == NULL) { base_dir = getenv("TMPDIR_ENV"); }
  if (base_dir == NULL) { base_dir = TMP_DIR; }
  
  Info.path=base_dir;
  Info.item_size=sizeof(SortItem);
  Info.num_items=APP_DEFAULT_N_ITEMS;
  Info.mem_size=APP_DEFAULT_MEM_SIZE;

  nopts=0;
  while ( (optidx=getopts(argc, argv, opts, &optarg)) != 0) {
    nopts++;
    if( optidx==-1 ){
      std::cerr << "Could not allocate space for arguments. Exiting...\n";
      exit(1);
    }
    if(optidx==APP_OPTION_PATH){
      Info.path=optarg;
    }
    else if(optidx==APP_OPTION_MEM_SIZE){
      tmp = static_cast<TPIE_OS_SIZE_T>(ascii2longlong(optarg));
      if(tmp < 0){
        std::cerr << "Invalid memory size. Exiting...\n";
        exit(1);
      }
      else { Info.mem_size=static_cast<TPIE_OS_SIZE_T>(tmp); }
    }
    else if(!optset){
      //set optset flag if applicable
      switch(optidx){
        case APP_OPTION_NUM_ITEMS:
        case APP_OPTION_FILE_SIZE:
          optset=true;
        default:
          break;
      }
      //do actual setup
      switch(optidx){
        case APP_OPTION_NUM_ITEMS:
          tmp = ascii2longlong(optarg);
          if(tmp < 0){
            std::cerr << "Invalid item count. Exiting...\n";
            exit(1);
          }
          else { Info.num_items=tmp; }
          break;
        case APP_OPTION_FILE_SIZE:
          tmp = ascii2longlong(optarg);
          if(tmp < 0){
            std::cerr << "Invalid file size. Exiting...\n";
            exit(1);
          }
          else { Info.num_items=(tmp/Info.item_size)+1; }
          break;
        default:
          std::cerr << "Warning: Unhandled option - " << optidx << std::endl;
          break;
      }
    } //else if(!optset)
    else {
      //optset=true, optidx != APP_OPTION_PATH
      //check for bad options
      switch(optidx){
        case APP_OPTION_NUM_ITEMS:
        case APP_OPTION_FILE_SIZE:
          print_warning();
          break;
        default:
          std::cerr << "Warning: Unhandled option - " << optidx << std::endl;
          break;
      } //switch
    } //else
  } //while options

  // if no command line options, display usage
  if(nopts == 0) { 
    // add more general comments here if neeeded
    // then display usage options
    printf("\nSummary: Writes a random stream of specified size\n"
           "to a temporary file, sorts the stream\n"
           "then deletes the input/output streams and exits.\n"
           "Useful for testing basic AMI_sort routine\n\n");
    getopts_usage(argv[0], opts);
    printf("\nEach item is %d bytes\n", APP_ITEM_SIZE);
    printf("--path-name is \"%s\" by default\n", TMP_DIR);
    printf("Suffixes K, M, and G can be appended to the\n"
           "--numitems and --size options to mean\n"
           "*1024, *1024*1024, and *2^30 respectively\n"
           "e.g., --size 128M creates a 128 MB stream\n\n");
    printf("Example runs\n\n");
    printf("Test a stream of size 2 GB\n");
    printf("%s -s 2G\n",argv[0]);
   
    exit(1);
  }  
  
  //check if path is valid
  tempname::set_default_base_name(APP_FILE_BASE);
  tempname::set_default_path(Info.path);
  std::string tmpfname = tempname::tpie_name();
  TPIE_OS_FILE_DESCRIPTOR fd;
  fd=TPIE_OS_OPEN_OEXCL(tmpfname, TPIE_OS_FLAG_USE_MAPPING_FALSE);
  if(TPIE_OS_IS_VALID_FILE_DESCRIPTOR(fd)){
    TPIE_OS_CLOSE(fd);
    TPIE_OS_UNLINK(tmpfname);
  }
  else{
    std::cerr << "Unable to write to path " << Info.path 
         << ".  Exiting..." << std::endl;
    exit(1);
  }
  
  delete [] opts; 
  return; 
}

//converts numbers into strings
//with appropriate G, M, K suffixes
char* ll2size(stream_offset_type n, char* buf){
  
  const int bufsize = 20;
  double size;
  if(n > APP_GIG ){
    size = (n*1.)/APP_GIG;
    APP_SNPRINTF(buf, bufsize, "%.2f G",size);
  }
  else if (n > APP_MEG ){
    size = (n*1.)/APP_MEG;
    APP_SNPRINTF(buf, bufsize, "%.2f M",size);
  } 
  else if (n > 1024 ){
    size = (n*1.)/1024.;
    APP_SNPRINTF(buf, bufsize, "%.2f K",size);
  }
  else {
    size = (n*1.);
    APP_SNPRINTF(buf,bufsize, "%.0f", size);
  }

  return buf;
}

// Open a stream, write num_items, close stream
void write_random_stream(std::string fname, appInfo & info, progress_indicator_base & indicator){
	TPIE_OS_OFFSET i,n; //,trunc;
  ami::err ae = ami::NO_ERROR;
  i=0;
  n=info.num_items;

  ami::stream<SortItem>* str = new ami::stream<SortItem>(fname);
  assert(str->is_valid());
  str->persist(PERSIST_PERSISTENT);

  //std::cout << "Opened file " << fname 
  //    << "\nWriting "<< n << " items..." << std::endl;
  // trunc=(static_cast<TPIE_OS_OFFSET>(sizeof (SortItem)))*n;
  // if(trunc<0 || trunc>(4*APP_GIG)){
  //   std::cout << "Initial file length computed as "<< trunc
  //        << "\nSetting to 4GB "<< std::endl;
  //   trunc=4*APP_GIG;
  // }
  // //AMI stream truncate is based on item count, not bytes
  // trunc/=sizeof(SortItem);
  // ae = str->truncate(trunc);
  // if(ae != ami::NO_ERROR){
  //   std::cout << "\nError truncating file"<< std::endl;
  // }
  str->seek(0);

  indicator.init(n);
  while((i<n) && (ae==ami::NO_ERROR)){
    ae=str->write_item(SortItem(TPIE_OS_RANDOM()));   
    i++;
	indicator.step();
  }

  indicator.done();
  
  if(ae != ami::NO_ERROR){
    std::cout<< "\nWrite stopped early with AMI_ERROR: " << ae << std::endl;
  }
  
  //std::cout << "\nWrote " << i << " items\n" << std::endl;
  delete str;
  TP_LOG_APP_DEBUG_ID("Returning from write_random_stream"); 
  return;
}

// Read sorted stream from fname and check that its elements are sorted
void check_sorted(std::string fname, appInfo & info, progress_indicator_base & indicator){

  stream_offset_type i,n;
  SortItem *x = 0, x_prev;
  ami::err ae=ami::NO_ERROR;
   
  n=info.num_items;

  //std::cout << "Checking that output is sorted." << std::endl;
  //TP_LOG_APP_DEBUG_ID("Checking that output is sorted"); 
  
  ami::stream<SortItem>* str = new ami::stream<SortItem>(fname);
  assert(str->is_valid());
  str->persist(PERSIST_PERSISTENT);
  str->seek(0);
  
  indicator.init(n);
  i=0;
  while((i<n) && (ae==ami::NO_ERROR)){
    ae=str->read_item(&x);
    i++;
    if(i>1){ 
      if(x_prev.key > x->key){
	  std::cerr << "prev = " << x_prev.key
	       << ", curr = " << x->key  
	       << ", i = " <<  i << std::endl;
      }
      tp_assert(x_prev.key <= x->key, 
                       "List not sorted! Exiting");
    }
    x_prev=*x;
	indicator.step();
  }

  indicator.done();
  
  if(ae != ami::NO_ERROR){
    std::cout<< "\nRead stopped early with AMI_ERROR: " << ae << std::endl;
  }
  
  //std::cout << "\nRead " << i << " items\n" << std::endl;
  delete str;
  return;
}

void load_list(ami::stream<SortItem>* str, SortItem* list, TPIE_OS_SIZE_T nitems){
  SortItem *s_item = 0;
  str->seek(0);
  for(TPIE_OS_SIZE_T i=0; i<nitems; i++){
    str->read_item(&s_item);
    assert(s_item != NULL);
    list[i]=*s_item;
  }
}

// // Internal sort tests
// void internal_sort_test(const appInfo& info){  
//   TPIE_OS_SIZE_T str_mem_usage;
//   TPIE_OS_SIZE_T i,nitems;
//   SortItem *list;
//   cpu_timer clk;
//   SortCompare cmp;
//   char buf[20];

//   tempname::set_default_base_name(APP_FILE_BASE);
//   tempname::set_default_path(info.path);
//   ami::stream<SortItem>* Str = new ami::stream<SortItem>();
  
//   Str->seek(0);
//   Str->main_memory_usage(&str_mem_usage, mem::STREAM_USAGE_MAXIMUM);
//   nitems=(MM_manager.memory_available()-str_mem_usage-16)/sizeof(SortItem);
//   list=new SortItem[nitems];
//   for(i=0; i<nitems; i++){
//     Str->write_item(SortItem(TPIE_OS_RANDOM()));   
//   }
//   std::cout << "Number items: " << nitems << " size: "
//        << ll2size(nitems*sizeof(SortItem),buf) << std::endl;
       
//   load_list(Str, list, nitems);
//   std::cout << "STL sort: ";
//   clk.reset();
//   clk.start();
//   std::sort(list, list+nitems);
//   clk.stop();
//   std::cout << clk.wall_time() << std::endl;
  
//   load_list(Str, list, nitems);
//   std::cout << "STL sort obj: ";
//   clk.reset();
//   clk.start();
//   std::sort(list, list+nitems, cmp);
//   clk.stop();
//   std::cout << clk.wall_time() << std::endl;
  
//   load_list(Str, list, nitems);
//   std::cout << "STL stable sort: ";
//   clk.reset();
//   clk.start();
//   std::stable_sort(list, list+nitems);
//   clk.stop();
//   std::cout << clk.wall_time() << std::endl;
  
//   load_list(Str, list, nitems);
//   std::cout << "STL stable sort obj: ";
//   clk.reset();
//   clk.start();
//   std::stable_sort(list, list+nitems, cmp);
//   clk.stop();
//   std::cout << clk.wall_time() << std::endl;
 
//   Str->persist(PERSIST_DELETE);
//   delete [] list;
//   delete Str;
// }

// ami::err test_3x_sort(appInfo& info, enum test_type ttype, progress_indicator_base* indicator=NULL){
// 	//Make up some temp filenames
// 	std::cout << "****TEST START****" << std::endl;
// 	execution_time_predictor global_predictor(
// 		((unique_id_generator()) << "test_sort.cpp" << "test_3x_sort")());
// 	if (indicator) indicator->set_time_predictor(&global_predictor);
	
// 	execution_time_predictor write_predictor( 
// 		((unique_id_generator()) << "test_sort.cpp" << "test_3x_sort" << "write")());
// 	execution_time_predictor sort_predictor( 
// 		((unique_id_generator()) << "test_sort.cpp" << "test_3x_sort" << "sort")());
// 	execution_time_predictor check_predictor( 
// 		((unique_id_generator()) << "test_sort.cpp" << "test_3x_sort" << "check")());
// 	fractional_time_predictor ftp;
// 	ftp.add_step(0.15, info.num_items, &write_predictor);
// 	ftp.add_step(0.70, info.num_items, &sort_predictor);
// 	ftp.add_step(0.15, info.num_items, &check_predictor);

// //  char fname[BUFSIZ], fname2[BUFSIZ];
// //  strncpy(fname, tpie_tempnam(APP_FILE_BASE, info.path), BUFSIZ);
// //  strncpy(fname2, tpie_tempnam(APP_FILE_BASE, info.path), BUFSIZ);
// 	tempname::set_default_base_name(APP_FILE_BASE);
// 	tempname::set_default_path(info.path);
// 	std::string fname  = tempname::tpie_name();
// 	std::string fname2 = tempname::tpie_name();
	
// 	write_predictor.start_execution(info.num_items);
// 	progress_indicator_subindicator write_progress(0, info.num_items, 1, indicator, ftp.get_fraction(&write_predictor) );
// 	write_random_stream(fname, info, &write_progress);
//   	write_predictor.end_execution();

// 	//Sort
// 	ami::err ae;
// 	ami::stream<SortItem>* inStr = new ami::stream<SortItem>(fname);
// 	ami::stream<SortItem>* outStr = new ami::stream<SortItem>(fname2);
// 	std::cout << "\nMem available: " << MM_manager.memory_available()
// 			  << "\nSorting "<< fname << " to " << fname2 << std::endl; 
// 	TP_LOG_APP_DEBUG_ID("Starting sort"); 
// 	SortCompare cmp;
// 	KeySortCompare kcmp;
// 	long dummykey = 0;
// 	sort_predictor.start_execution(info.num_items);
//   switch(ttype){
//     case APP_TEST_OBJ_OP:
//       std::cout << "Using operator sorting and object heaps" << std::endl;
//       ae=ami::sort(inStr, outStr, indicator);
//       break;
//     case APP_TEST_PTR_OP:
//       std::cout << "Using operator sorting and ptr heaps" << std::endl;
//       ae=ami::ptr_sort(inStr, outStr, indicator);
//       break;
//     case APP_TEST_OBJ_CMPOBJ: 
//       std::cout << "Using comp obj sorting and object heaps" << std::endl;
//       ae=ami::sort(inStr, outStr, &cmp, indicator);
//       break;
//     case APP_TEST_PTR_CMPOBJ: 
//       std::cout << "Using comp obj sorting and ptr heaps" << std::endl;
//       ae=ami::ptr_sort(inStr, outStr, &cmp, indicator);
//       break;
//     case APP_TEST_KOBJ: 
//       std::cout << "Using key+obj sorting and object heaps" << std::endl;
//       ae=ami::key_sort(inStr, outStr, dummykey, &kcmp, indicator);
//       break;
//     default:
//       ae=ami::GENERIC_ERROR;
//       break;
//   }
//   sort_predictor.end_execution();
//   TP_LOG_APP_DEBUG_ID("Done with sort"); 
//   if(ae != ami::NO_ERROR){
//     std::cout << "Error during sorting: ";
//     switch(ae){
//       case ami::INSUFFICIENT_MAIN_MEMORY:
//         std::cout << "insufficient memory. Brother, can you spare a meg?";
//         break;
//       default:
//         std::cout << "AE code " << ae << " look this number up in ami_err.h";
//     }
//     std::cout << std::endl;
//   }
//   std::cout << "Input stream length = " << inStr->stream_len() << std::endl;
//   std::cout << "Output stream length = " << outStr->stream_len() << std::endl;
//   inStr->persist(PERSIST_PERSISTENT);
//   outStr->persist(PERSIST_PERSISTENT);
//   delete inStr;
//   delete outStr;

//   //Check the output
//   if(ae==ami::NO_ERROR) { 
// 	  check_predictor.start_execution(info.num_items);
//       check_sorted(fname2, info, indicator); 
// 	  check_predictor.end_execution();
//   }

//   //delete stream from disk
//   std::cout << "\nDeleting streams " << fname << " and " << fname2 << std::endl;
//   TPIE_OS_UNLINK(fname);
//   TPIE_OS_UNLINK(fname2);

//   std::cout << "****TEST STOP****\n" << std::endl;
//   return ae;
// }

ami::err test_2x_sort(appInfo& info, enum test_type ttype, progress_indicator_base & indicator){
  //Make up some temp filenames
	//std::cout << "****TEST START****" << std::endl;
//  char fname[BUFSIZ];
//  strncpy(fname, tpie_tempnam(APP_FILE_BASE, info.path), BUFSIZ);
	std::string fname = tempname::tpie_name();
	
	fractional_progress fp(&indicator);
	fp.id() << __FILE__ << __FUNCTION__ << ttype;
	fractional_subindicator write_progress(fp, "write", TPIE_FSI, info.num_items, "Writing random input");
	fractional_subindicator sort_progress(fp, "sort", TPIE_FSI, info.num_items, "Sorting");
	fractional_subindicator check_progress(fp, "check", TPIE_FSI, info.num_items, "Checking");
	fp.init();
	write_random_stream(fname, info, write_progress);
  
	//Sort
	ami::err ae;
	ami::stream<SortItem>* inStr = new ami::stream<SortItem>(fname);
	SortCompare cmp;
	KeySortCompare kcmp;
	switch(ttype){
    case APP_TEST_OBJ_OP:
//      std::cout << "Using operator sorting and object heaps" << std::endl;
		ae=ami::sort(inStr, (progress_indicator_base*)&sort_progress);
		break;
    case APP_TEST_PTR_OP:
		//     std::cout << "Using operator sorting and ptr heaps" << std::endl;
      ae=ami::ptr_sort(inStr, (progress_indicator_base*)&sort_progress);
      break;
    case APP_TEST_OBJ_CMPOBJ: 
//      std::cout << "Using comp obj sorting and object heaps" << std::endl;
      ae=ami::sort(inStr, &cmp, (progress_indicator_base*)&sort_progress);
      break;
    case APP_TEST_PTR_CMPOBJ: 
//      std::cout << "Using comp obj sorting and ptr heaps" << std::endl;
      ae=ami::ptr_sort(inStr, &cmp, (progress_indicator_base*)&sort_progress);
      break;
    default:
      ae=ami::GENERIC_ERROR;
      break;
  }

	TP_LOG_APP_DEBUG_ID("Done with sort"); 
  if(ae != ami::NO_ERROR){
    std::cout << "Error during sorting: ";
    switch(ae){
      case ami::INSUFFICIENT_MAIN_MEMORY:
        std::cout << "insufficient memory. Brother, can you spare a meg?";
        break;
      default:
        std::cout << "AE code " << ae << " look this number up in ami_err.h";
    }
    std::cout << std::endl;
  }
  inStr->persist(PERSIST_PERSISTENT);
  delete inStr;

  //Check the output
  if(ae==ami::NO_ERROR){ check_sorted(fname, info, check_progress); }
  //delete stream from disk
  TPIE_OS_UNLINK(fname);
  fp.done();
  return ae;
}

int main(int argc, char** argv){
  tpie_init();
  appInfo info;
  char buf[20];
  TPIE_OS_SRANDOM(time(NULL));
  get_app_info(argc, argv, info);

  //Set up TPIE memory manager
  get_memory_manager().set_limit(info.mem_size);

  //Set up TPIE logging. 
  //Log files will be written to /tmp/tpielog_XXXXXX.txt
  //where XXXXX is randomly generated

  //tpie_log_init(TPIE_LOG_MEM_DEBUG);
  //TP_LOG_SET_THRESHOLD(TPIE_LOG_MEM_DEBUG);
  //printf("Log file is %s\n", tpie_log_name());

  stream_offset_type filesize = info.num_items*info.item_size;
  std::cout << "Path:  " << info.path 
       << "\nNum Items: " << info.num_items 
       << "\nItem Size: " << info.item_size
       << "\nFile Size: " << ll2size(filesize,buf) << "B\n" <<std::endl;

  progress_indicator_arrow myIndicator("Testing sorting", 10000);
  
  execution_time_predictor global_predictor(((unique_id_type()) << __FILE__ << __FUNCTION__)());
  myIndicator.set_time_predictor(&global_predictor);
  global_predictor.start_execution(info.num_items );
  ami::err ae=ami::NO_ERROR;
//   std::cout << "++++start 3X space tests++++" << std::endl;
// #if 0
//   if(ae==ami::NO_ERROR){
//     ae=test_3x_sort(info, APP_TEST_OBJ_OP, myIndicator);
//   }
//   if(ae==ami::NO_ERROR){
//     ae=test_3x_sort(info, APP_TEST_PTR_OP, myIndicator);
//   }
//   if(ae==ami::NO_ERROR){
//     ae=test_3x_sort(info, APP_TEST_OBJ_CMPOBJ, myIndicator);
//   }
//   if(ae==ami::NO_ERROR){
//     ae=test_3x_sort(info, APP_TEST_PTR_CMPOBJ, myIndicator);
//   }
//   if(ae==ami::NO_ERROR){
//     ae=test_3x_sort(info, APP_TEST_KOBJ, myIndicator);
//   }
// #endif
//   std::cout << "++++end 3X space tests++++" << std::endl;
//   std::cout << "++++start 2X space tests++++" << std::endl;
  if(ae==ami::NO_ERROR){
	  ae=test_2x_sort(info, APP_TEST_OBJ_OP, myIndicator);
  }
// #if 0
//   if(ae==ami::NO_ERROR){
//     ae=test_2x_sort(info, APP_TEST_PTR_OP, myIndicator);
//   }
//   if(ae==ami::NO_ERROR){
//     ae=test_2x_sort(info, APP_TEST_OBJ_CMPOBJ, myIndicator);
//   }
//   if(ae==ami::NO_ERROR){
//     ae=test_2x_sort(info, APP_TEST_PTR_CMPOBJ, myIndicator);
//   }
//   if(ae==ami::NO_ERROR){
//     ae=test_2x_sort(info, APP_TEST_KOBJ, myIndicator);
//   }
// #endif
//   std::cout << "++++end 2X space tests++++" << std::endl;

// #if 0
//   std::cout << "Internal sort testing..." << std::endl;
//   internal_sort_test(info);
// #endif

  global_predictor.end_execution();
  if(ae==ami::NO_ERROR){ std::cout << "Test ran successfully " << std::endl; }
  else { std::cout << "Test at least ran without crashing" << std::endl; }

  return 0;
}
