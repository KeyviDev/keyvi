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

// Copyright (c) 1999 Jan Vahrenhold
//
// File:         joinlog.cpp
// Author:       Jan Vahrenhold <jan@cs.duke.edu>
// Created:      01/24/99
// Description:  
//
// $Id: joinlog.cpp,v 1.2 2004-08-12 12:39:44 jan Exp $
//

#include <string.h>
#include <iomanip>
#include "joinlog.h"

#define COPYSTRING(target,source) \
      if (strstr(source,".stream") != NULL) {             \
         char* occur = strstr(source,".stream");          \
         target = new char[(occur-source)+1];             \
         strncpy(target, source, (size_t)(occur-source)); \
         target[occur-source] = '\0';                     \
      }                                                   \
      else {                                              \
         target = new char[strlen(source)+1];             \
         strcpy(target, source);                          \
      };

#ifndef _WIN32
static struct rusage Usage;
static struct timeval UsageTime;
#endif

JoinLog::JoinLog() : program_(NULL), redName_(NULL), blueName_(NULL), redLength_(0), blueLength_(0), fanOut_(0) {
}

JoinLog::JoinLog(const char* program, const char* redName, TPIE_OS_OFFSET redLength, const char* blueName, TPIE_OS_OFFSET blueLength, unsigned short fanOut) {
    COPYSTRING(program_,program);

    COPYSTRING(blueName_,blueName);
    blueLength_ = blueLength;

    COPYSTRING(redName_,redName);
    redLength_ = redLength;

    fanOut_ = fanOut;
}

JoinLog::~JoinLog() {
    delete[] program_;
    delete[] redName_;
    delete[] blueName_;
}

void JoinLog::UsageStart() {
#ifndef _WIN32
  gettimeofday(&UsageTime,NULL);
  getrusage(RUSAGE_SELF, &Usage);
  MemUsageStart();
#endif
}

void JoinLog::subtimeval(struct timeval* diff, struct timeval* t1, struct timeval* t2) {
#ifndef _WIN32
    diff->tv_sec = t1->tv_sec - t2->tv_sec;
    diff->tv_usec = t1->tv_usec - t2->tv_usec;
    if (diff->tv_usec < 0)  {
	diff->tv_usec += 1000000;
	diff->tv_sec--;
    }
#endif
}

void JoinLog::UsageEnd(const char *where, TPIE_OS_OFFSET resultLength) {

#ifndef _WIN32
	struct rusage usage;
  struct timeval diff;
  struct timeval now;
  double sys_time, user_time, wall_time;

  gettimeofday(&now,NULL);
  getrusage(RUSAGE_SELF, &usage);

  subtimeval (&diff, &now, &UsageTime);

  if (where) {
      cerr << where << endl;
  }

  cout << setw(4) << program_ << " ";
#ifdef BTE_IMP_MMB
  cout << "M " << setw(2) << BTE_MMB_LOGICAL_BLOCKSIZE_FACTOR << setw(0) << " ";
#endif
#ifdef BTE_IMP_STDIO
  cout << "S " << setw(2) << "--" << setw(0) << " ";
#endif
#ifdef BTE_IMP_UFS
  cout << "U " << setw(2) << BTE_UFS_LOGICAL_BLOCKSIZE_FACTOR << setw(0) << " ";
#endif
  cout << setw(3) << fanOut_ << setw(0) << " ";
  cout << setw(3) << test_mm_size / (1024 * 1024) << setw(0) << " ";
  cout << setw(10) << redName_ << setw(0) << " ";
  cout << setw(8) << redLength_ << setw(0) << " "; 
  cout << setw(10) << blueName_ << setw(0) << " ";
  cout << setw(8) << blueLength_ << setw(0) << " "; 
  cout << setw(8) << resultLength << setw(0) << " "; 

  //  cout.precision(2);
  //  cout.setf(ios::scientific, ios::floatfield);

  cerr << diff.tv_sec << setw(0) << ".";
  cerr << setw(6) << setfill('0') << diff.tv_usec << setfill(' ') << setw(0);
  cerr << " -- elapsed time" << endl;
  //  cout << setw(6) << diff.tv_sec << setw(0) << ".";
  //  cout << setfill('0') << setw(6) << diff.tv_usec << setfill(' ') << setw(0) << " ";
  wall_time =  diff.tv_sec; // + (double) diff.tv_usec / 1000000.0;
  cout << setw(4) << (int) wall_time << setw(0) << " ";

  subtimeval (&diff, &usage.ru_utime, &Usage.ru_utime);
  cerr << diff.tv_sec << setw(0) << ".";
  cerr << setw(6) << setfill('0') << diff.tv_usec << setfill(' ') << setw(0);
  cerr << " -- user time" << endl;
  //  cout << setw(6) << diff.tv_sec << setw(0) << ".";
  //  cout << setfill('0') << setw(6) << diff.tv_usec << setfill(' ') << setw(0) << " ";
  user_time = diff.tv_sec;// + (double) diff.tv_usec / 1000000.0;

  subtimeval (&diff, &usage.ru_stime, &Usage.ru_stime);
  cerr << diff.tv_sec << setw(0) << ".";
  cerr << setw(6) << setfill('0') << diff.tv_usec << setfill(' ') << setw(0);
  cerr << " -- system time" << endl;
  //  cout << setw(6) << diff.tv_sec << setw(0) << ".";
  //  cout << setfill('0') << setw(6) << diff.tv_usec << setfill(' ') << setw(0) << " ";
  sys_time =  diff.tv_sec;// + (double) diff.tv_usec / 1000000.0;
  cout << setw(4) << (int) (user_time + sys_time) << setw(0) << " ";

  cout << setw(4) << setprecision(1) << 100 - (int) ((100*(sys_time+user_time))/wall_time) << setw(0) << "% ";

  cout << setw(7) << usage.ru_inblock;
  cout << endl;

  usage.ru_majflt -= Usage.ru_majflt;
  cerr << usage.ru_majflt << " -- phys page faults" << endl; ; 
  usage.ru_nswap -= Usage.ru_nswap;
  cerr << usage.ru_nswap << " -- swaps" << endl;
  usage.ru_inblock -= Usage.ru_inblock;
  cerr << usage.ru_inblock << " -- data in" << endl; 
  usage.ru_oublock -= Usage.ru_oublock;
  cerr << usage.ru_oublock << " -- data out" << endl;
  usage.ru_msgsnd -= Usage.ru_msgsnd;
  cerr << usage.ru_msgsnd << " -- msg send" << endl;
  usage.ru_msgrcv -= Usage.ru_msgrcv;
  cerr << usage.ru_msgrcv << " -- msg recv" << endl;
  usage.ru_nvcsw -= Usage.ru_nvcsw;
  cerr << usage.ru_nvcsw << " -- vol ctx sw" << endl;
  usage.ru_nivcsw -= Usage.ru_nivcsw;
  cerr << usage.ru_nivcsw << " -- invol ctx sw" << endl;
  MemUsageEnd();
#else
	cerr << "UsageEnd(const char*, TPIE_OS_OFFSET) is not implemented for this platform." << endl;
#endif
}

void JoinLog::UsageEndBrief(const char* where) {

#ifndef _WIN32

  struct rusage usage;
  struct timeval diff;
  struct timeval now;

  gettimeofday(&now,NULL);
  getrusage(RUSAGE_SELF, &usage);

  subtimeval (&diff, &now, &UsageTime);

  if (where) {
      cerr << where << endl;
  }
  cerr << diff.tv_sec << setw(6) << diff.tv_usec << setw(0) << " -- elapsed time" << endl;
  MemUsageEnd();
#else
	cerr << "UsageEndBrief(const char*) is not implemented for this platform." << endl;
#endif
}

/* static prstatus_t mystatus;
static int procfd = 0;
*/
#ifndef _WIN32
static unsigned long initialBrkValue;
#endif 

/*
void JoinLog::getMemUsage() {
  char mypidstr[100];
  int retval;

  if (!procfd) {
	 sprintf(mypidstr, "/proc/%5.5d", getpid());
	 procfd = open(mypidstr, O_RDONLY);
  }
  retval = ioctl(procfd, PIOCSTATUS, &mystatus);
}
*/

void JoinLog::MemUsageStart() {
  /*
  char mypidstr[100];
  int retval;
  
  if (!procfd) {
      sprintf(mypidstr, "/proc/%5.5d", getpid());
      procfd = open(mypidstr, O_RDONLY);
  }

  retval = ioctl(procfd, PIOCSTATUS, &mystatus);
  */

#ifndef _WIN32
  initialBrkValue = (unsigned long) sbrk(0);  
#endif 
}

void JoinLog::MemUsageEnd() {
  /*
  prstatus_t newstatus;
  int retval;

  retval = ioctl(procfd, PIOCSTATUS, &newstatus);
  fprintf(stdout, "Heap grew by %d bytes\n", newstatus.pr_brksize -
      mystatus.pr_brksize);
  */

#ifndef _WIN32
  unsigned long diff = (unsigned long) sbrk(0) - initialBrkValue;
  cerr << "Heap grew by " << diff << " bytes\n";
#endif
}

