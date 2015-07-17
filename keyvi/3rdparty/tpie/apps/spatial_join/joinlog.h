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
// File:         joinlog.h
// Author:       Jan Vahrenhold <jan@cs.duke.edu>
// Created:      01/24/99
// Description:  
//
// $Id: joinlog.h,v 1.3 2004-08-12 18:01:50 jan Exp $
//
#ifndef JOINLOG_H
#define JOINLOG_H

#include <portability.h>

// Most of the statistics do not work on WIN32 platforms.
#ifndef _WIN32

#include <sys/types.h>
#include <sys/signal.h>
//#include <sys/fault.h>
#include <sys/syscall.h>
//#include <sys/procfs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#ifdef __FreeBSD__
#include <sys/ioctl.h>
#include <sys/pioctl.h>
#endif

#ifdef undefined
struct    rusage {
    struct timeval ru_utime;      // user time used 
    struct timeval ru_stime;      // system time used 
    int  ru_maxrss;               // maximum resident set size 
    int  ru_ixrss;                // currently 0 
    int  ru_idrss;                // integral resident set size 
    int  ru_isrss;                // currently 0 
    int  ru_minflt;               // page faults not requiring physical I/O 
    int  ru_majflt;               // page faults requiring physical I/O 
    int  ru_nswap;                // swaps 
    int  ru_inblock;              // block input operations 
    int  ru_oublock;              // block output operations 
    int  ru_msgsnd;               // messages sent 
    int  ru_msgrcv;               // messages received 
    int  ru_nsignals;             // signals received 
    int  ru_nvcsw;                // voluntary context switches 
    int  ru_nivcsw;               // involuntary context switches 
};
#endif

#endif // ifndef _WIN32

//  Do not remove this #include !!!
#include "app_config.h"

//- JoinLog
class JoinLog {
//.  The class JoinLog serves as a wrapper for routines from the file "usage.c"
//.  and sends a formatted output of the statistics to 'cout'. The usual
//.  output will be sent to 'cerr'.
public:
    
    //- constructor, destructor
    JoinLog();
    JoinLog(
	const char*    program, 
	const char*    redName, 
	TPIE_OS_OFFSET redLength, 
	const char*    blueName, 
	TPIE_OS_OFFSET blueLength, 
	unsigned short fanOut = 0);
    ~JoinLog();
    //.  The constructor expects the identifier of the program from which it
    //.  is called, name and length of both input streams, and (in case of
    //.  R-trees) the fanout of the tree.

    //- UsageStart, UsageEnd, UsageEndBrief
    void UsageStart();
    void UsageEnd(
	const char*   where = NULL,
	TPIE_OS_OFFSET resultLength = 0);
    void UsageEndBrief(const char* where = NULL);
    //.  UsageStart starts recording statistics, whereas UsageEnd and 
    //.  UsageEndBrief stop recording and print the statistics.

    //- setRedLength, setBlueLength
    void setRedLength(TPIE_OS_OFFSET redSize);
    void setBlueLength(TPIE_OS_OFFSET redSize);
    //.  The size of the input data can be explicitly set. This is 
    //.  necessary for working with "sortjoin".

protected:

    void subtimeval(struct timeval* diff, struct timeval* t1, struct timeval* t2);
  //    void getMemUsage();
    void MemUsageStart();   
    void MemUsageEnd();

    char*          program_;     //  Identifier of the program.
    char*          redName_;     //  Red input stream.
    TPIE_OS_OFFSET redLength_;   //  Number of red objects.
    char*          blueName_;    //  Blue input stream.
    TPIE_OS_OFFSET blueLength_;  //  Number of blue objects.
    unsigned short fanOut_;      //  Fanout of the tree.

private:

};

inline void JoinLog::setRedLength(TPIE_OS_OFFSET redLength) {
    redLength_ = redLength;
}

inline void JoinLog::setBlueLength(TPIE_OS_OFFSET blueLength) {
    blueLength_ = blueLength;
}

#endif

//
//   End of File.
//

