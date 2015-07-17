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
// File:         sorting_adaptor.cpp
// Author:       Jan Vahrenhold <jan@cs.duke.edu>
// Created:      01/24/99
// Description:  
//
// $Id: sorting_adaptor.cpp,v 1.3 2005-11-10 10:35:57 adanner Exp $
//

#include <iostream>
using std::cerr;
using std::endl;
using std::istream;
using std::ostream;
#include "sorting_adaptor.h"
#include <ami_sort.h>

SortingAdaptor::SortingAdaptor(const char* inStreamName) {
    currentRect_ = new rectangle;
    nextRect_    = NULL;

    char* sortedName = new char[strlen(inStreamName)+strlen(".sorted")+1];
    strcpy(sortedName, inStreamName);
    strcat(sortedName, ".sorted");
      
    AMI_STREAM<rectangle> *unsortedStream = new AMI_STREAM<rectangle>(inStreamName);
    unsortedStream->persist(PERSIST_PERSISTENT);

    size_ = unsortedStream->stream_len();

    sortedStream_ = new AMI_STREAM<rectangle>(sortedName);
    sortedStream_->persist(PERSIST_PERSISTENT);

    coord_t dummyKey = 0;
    //rectangle r_dummyKey;
    //  Sort the rectangles in the input stream according to ylo.
    AMI_sort(unsortedStream, sortedStream_);
    //AMI_partition_and_merge_stream(unsortedStream, sortedStream_);
    delete unsortedStream;
    sortedStream_->seek(0);

    //  Read the first item from the stream.
    AMI_err result = sortedStream_->read_item(&nextRect_);

    if (result != AMI_ERROR_NO_ERROR) {
	cerr << "Error while using " << sortedName << " (";
	cerr  << result << ")." << endl;
    }

	delete[] sortedName;
}

SortingAdaptor::SortingAdaptor(const SortingAdaptor& other) {
    *this = other;
}

SortingAdaptor& SortingAdaptor::operator=(const SortingAdaptor& other) {
    if (this != &other) {
	cerr << "Please create a new sorting adaptor instead of ";
	cerr << "copying it." << endl;
    }
    return (*this);
}

SortingAdaptor::~SortingAdaptor() {

    sortedStream_->persist(PERSIST_DELETE);

    delete currentRect_;
    delete sortedStream_;
}

//
//   End of File.
//

