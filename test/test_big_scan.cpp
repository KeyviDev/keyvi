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
#include <tpie/portability.h>
#include "test_portability.h"
#include <tpie/stream.h>
#include "getopts.h"

#include <cstring>
#include <tpie/memory.h>
#include <tpie/types.h>
#include <iostream>
#include <tpie/pretty_print.h>

using namespace tpie;

enum app_options { 
    APP_OPTION_PATH = 1,
    APP_OPTION_2GIG_TEST,
    APP_OPTION_4GIG_TEST,
    APP_OPTION_NUM_ITEMS,
    APP_OPTION_FILE_SIZE,
    APP_OPTION_EMPTY,
};

const unsigned int APP_OPTION_NUM_OPTIONS=6; 
const stream_size_type APP_MEG = 1024*1024;
const stream_size_type APP_GIG = 1024*1024*1024;
const stream_size_type APP_DEFAULT_N_ITEMS = 10000;
const int  APP_ITEM_SIZE = 64;

const char* APP_FILE_BASE =  "TPIE_Test";

// Basic parameters for the test
// better than having several global variables
typedef struct app_info{
    const char * path;
    int item_size;
    stream_size_type num_items;
} appInfo;

// ********************************************************** 
// class Item
// **********************************************************

// A class that contains a simple fixed-size array of chars 
// The array can also be used as a counter in base-26 arithmetic
class Item{
public:
    Item();
    ~Item(){};
    stream_size_type convert();
    Item operator ++(int j);
    friend std::ostream& operator << (std::ostream & out, const Item & it);
    friend bool operator == (const Item & i1, const Item & i2);
private:
    char item[APP_ITEM_SIZE];
};

// Constructor
// Initialize item to base-26 equivalent of 0='....aaaaaaa'_26
Item::Item(){
    int i;
    for(i=0; i<APP_ITEM_SIZE-1; i++) item[i]='a';
    //null-terminate so we can print array as string
    item[APP_ITEM_SIZE-1]='\0';
}

//Converts ascii counter into actual numeric value
//assuming base 26 arithmetic
//assumes stream_size_type is 64 bits
stream_size_type Item::convert(){
    int i, stop;
    stream_size_type ans=0;
    //13 'digits' in base 26 ('zzzzzzzzzzzzz') is 
    //most digits that won't overflow a stream_size_type
    stop = (APP_ITEM_SIZE < 14) ? APP_ITEM_SIZE : 14;
    i=APP_ITEM_SIZE-stop;
    while(i < APP_ITEM_SIZE-1){
	ans = 26*ans+(item[i]-'a');
	i++;
    }
    return ans;
}

// A simple increment operator
Item Item::operator ++(int /* j */){
    int i = APP_ITEM_SIZE-2;

    //carry 
    while( (i > 0) && (item[i]=='z') ){
	item[i]='a';
	i--;
    }
  
    //increment first non-carry
    if(i>=0){ item[i]++;}
    return *this;
}
 
bool operator == (const Item & it1, const Item & it2){
    int i=0;
    while( i<APP_ITEM_SIZE ){
	if(it1.item[i] != it2.item[i]){ return false; }
	i++;
    }
    return true;
}

// An output operator. For large arrays, displaying only the
// last few characters should suffice
std::ostream & operator << (std::ostream & out, const Item & it){
  
    //display only this many characters (at most)
    const int len = 10;
  
    if(APP_ITEM_SIZE < len){
	return out << it.item;
    }
    else{
	return out << &(it.item[APP_ITEM_SIZE-len]);
    }
}   
// ********************************************************** 
// End Class Item
// ********************************************************** 

// Just like atoi or atol, but should also work for 64bit numbers
// Also supports KMG suffixes (e.g. 2K = 2*1024)

stream_offset_type ascii2longlong(char *s){

    TPIE_OS_SIZE_T i, len, multfactor;
    TPIE_OS_SSIZE_T digit;
    stream_offset_type val;
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
    opts[i].number=APP_OPTION_2GIG_TEST;
    opts[i].name = "2gig-test";
    opts[i].shortName = "g";
    opts[i].args = 0;
    opts[i].description = "Test files larger than 2 GB";

    i++;
    opts[i].number=APP_OPTION_4GIG_TEST;
    opts[i].name = "4gig-test";
    opts[i].shortName = "G";
    opts[i].args = 0;
    opts[i].description = "Test files larger than 4 GB";
    
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
	std::cerr << "Warning: Only one of --2gig-test, --4gig-test\n"
		  << "--numitems, or --size can be specified.\n"
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

    init_opts(opts, argc, argv); 
  
    Info.path=TMP_DIR;
    Info.item_size=APP_ITEM_SIZE;
    Info.num_items=APP_DEFAULT_N_ITEMS;

    nopts=0;
    while ( (optidx=getopts(argc, argv, opts, &optarg)) != 0 ){
	nopts++;
	if(optidx==-1){
	    std::cerr << "Could not allocate space for arguments. Exiting...\n";
	    exit(1);
	}

	if(optidx==APP_OPTION_PATH){
	    Info.path=optarg;
	}
	else if(!optset){
	    //set optset flag if applicable
	    switch(optidx){
	    case APP_OPTION_2GIG_TEST:
	    case APP_OPTION_4GIG_TEST:
	    case APP_OPTION_NUM_ITEMS:
	    case APP_OPTION_FILE_SIZE:
		optset=true;
	    default:
		break;
	    }
	    //do actual setup
	    stream_offset_type tmp;
	    switch(optidx){
	    case APP_OPTION_2GIG_TEST:
		Info.num_items=2*((APP_GIG/Info.item_size)+1);
		break;
	    case APP_OPTION_4GIG_TEST:
		Info.num_items=4*((APP_GIG/Info.item_size)+1);
		break;
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
	    case APP_OPTION_2GIG_TEST:
	    case APP_OPTION_4GIG_TEST:
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
	printf("\nSummary: Writes a stream of specified size\n"
	       "to a temporary file, reads back the stream\n"
	       "then deletes the stream and exits.\n"
	       "Useful for testing support of\n"
	       "TPIE files larger than 2GB and 4GB\n\n");
	getopts_usage(argv[0], opts);
	printf("\nEach item is %d bytes\n", APP_ITEM_SIZE);
	printf("--path-name is \"%s\" by default\n", tpie::tempname::get_default_path().c_str());
	printf("Suffixes K, M, and G can be appended to the\n"
	       "--numitems and --size options to mean\n"
	       "*1024, *1024*1024, and *2^30 respectively\n"
	       "e.g., --size 128M creates a 128 MB stream\n\n");
	printf("Example runs\n\n");
	printf("Test a stream of size 2 GB\n");
	printf("%s -g\n",argv[0]);
   
	exit(1);
    }  
  
    //check if path is valid
	tempname::set_default_path(Info.path);
//  tempname::set_default_base_name(APP_FILE_BASE);
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

// A simple progress bar that shows percentage complete
// and current file offset
void progress_bar(float pct, stream_size_type nbytes){

    const int nticks=20;
  
    int intpct, i;
  
    //percent done as an integer
    intpct = static_cast<int>(100*pct);
  
    std::cout <<"\r[";
    i=1;
    while( i <= (nticks*pct) ){
	std::cout << ".";
	i++;
    }
    while( i <= nticks ){
	std::cout << " ";
	i++;
    }
    std::cout <<"] "<<intpct<<"%";
 
    std::cout << " - " << bits::pretty_print::size_type(nbytes) << "      ";
    std::cout.flush();
}

// Open a stream, write num_items, close stream
void write_test(const std::string& fname, appInfo & info){
    
    TPIE_OS_OFFSET i,n;
    stream_size_type trunc_bytes;
    Item x;
    ami::err ae = ami::NO_ERROR;
    
    i=0;
    n=info.num_items;
    
    std::cout << "Starting write test." << std::endl;
  
    ami::stream<Item>* str = new ami::stream<Item>(fname);
    assert(str->is_valid());
    str->persist(PERSIST_PERSISTENT);
  
    std::cout << "Opened file " 
	      << fname 
	      << "\nWriting "
	      << n 
	      << " items..." << std::endl;
  
    trunc_bytes=sizeof (x)*n;
    if(trunc_bytes>(4*APP_GIG)){
	std::cout << "Initial file length computed as "<< trunc_bytes
		  << "\nSetting to 4GB "<< std::endl;
	trunc_bytes=4*APP_GIG;
    }
    ae = str->truncate(trunc_bytes / sizeof (x));
    if(ae != ami::NO_ERROR){
	std::cout << "\nError truncating file"<< std::endl;
    }
    str->seek(0);

    float pct = 0.;
    progress_bar(pct, 0);
    while((i<n) && (ae==ami::NO_ERROR)){
	ae=str->write_item(x);   
	x++;
	i++;
	if( (( i/(n*1.) ) - pct) > 0.001 ){
	    pct = i/(n*1.0f);
	    progress_bar(pct, i*info.item_size);
	}
    }
  
    pct = i/(n*1.0f);
    progress_bar(pct, i*info.item_size);
  
    if(ae != ami::NO_ERROR){
	std::cout<< "\nWrite stopped early with AMI_ERROR: " << ae << std::endl;
    }
  
    std::cout << "\nWrote " << i 
	      << " items\n" << "Closing file...";
    std::cout.flush();
    delete str;
    std::cout << "done" << std::endl;
    return;
}

void read_test(const std::string& fname, appInfo & info){

    stream_size_type i,n;
    Item *x1 = NULL;
    Item x2;
    ami::err ae=ami::NO_ERROR;
   
    i=0;
    n=info.num_items;

    std::cout << "Starting read test." << std::endl;
  
    ami::stream<Item>* str = new ami::stream<Item>(fname);
    assert(str->is_valid());
    str->persist(PERSIST_PERSISTENT);
    str->seek(0);
  
    std::cout << "Opened file " << fname 
	      << "\nReading "<< n << " items..." << std::endl;
  
    float pct = 0.;
    progress_bar(pct, 0);
    while((i<n) && (ae==ami::NO_ERROR)){
	ae=str->read_item(&x1);
	assert((*x1)==x2);
	x2++;
	i++;
	if( (( i/(n*1.) ) - pct) > 0.001 ){
	    pct = i/(n*1.0f);
	    progress_bar(pct, i*info.item_size);
	}
    }

    pct = i/(n*1.0f);
    progress_bar(pct, i*info.item_size);
  
    if(ae != ami::NO_ERROR){
	std::cout<< "\nRead stopped early with AMI_ERROR: " << ae << std::endl;
    }
  
    std::cout << "\nRead " << i 
	      << " items\n" << "Closing file...";
    std::cout.flush();
    delete str;
    std::cout << "done" << std::endl;
    return;
}

int main(int argc, char **argv){
    tpie_init();
    appInfo info;
    Item x;

    //Set up TPIE memory manager
    get_memory_manager().set_limit(64*APP_MEG);
    get_memory_manager().set_enforcement(memory_manager::ENFORCE_THROW);

    get_app_info(argc, argv, info);
 
    stream_size_type filesize = info.num_items*info.item_size;
    std::cout << "Path:  " << info.path 
	      << "\nNum Items: " << info.num_items 
	      << "\nItem Size: " << info.item_size
	      << "\nFile Size: " << bits::pretty_print::size_type(filesize) << std::endl;
 
	tempname::set_default_base_name(APP_FILE_BASE);
	tempname::set_default_path(info.path);
	std::string fname = tempname::tpie_name();
    write_test(fname, info);
    std::cout << std::endl;
    read_test(fname, info);

    //delete stream from disk
    std::cout << "\nDeleting stream " << fname << std::endl;
    TPIE_OS_UNLINK(fname);
  
    std::cout << "Test ran successfully " << std::endl;
    tpie_finish();
    return 0;
}
