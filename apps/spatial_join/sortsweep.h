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

// Copyright (c) 2002 Octavian Procopiuc
//
// File:         sortsweep.h
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      01/24/99
// Description:  Internal sweep class.
//
// $Id: sortsweep.h,v 1.2 2004-08-12 12:38:53 jan Exp $
//
#ifndef _SORTSWEEP_H
#define _SORTSWEEP_H

static const int DELETE_FREQUENCY = 10;

#include <portability.h>

//#include "intertree.h"
#include "sorting_adaptor.h"

class sort_sweep {
private:
  rectangle *prect;
  int crect, ncrect;
  AMI_err err;
  int counter;
  rectangle mbr_;

protected:
  SortingAdaptor *adaptor_[2];
  //  InterTree *iTree_[2];
  AMI_STREAM<pair_of_rectangles> *outStream_;
  TPIE_OS_OFFSET intersection_count;

public:
  sort_sweep(const char* red_filename, const char* blue_filename, 
	     AMI_STREAM<pair_of_rectangles> *outStream);
  ~sort_sweep();
  // run() takes the smallest item from the adaptor and runs 
  // the sweepline algorithm on it.
  AMI_err run();
  TPIE_OS_OFFSET getIntersectionCount() { return intersection_count; }
  TPIE_OS_OFFSET redSize() const { return adaptor_[0]->size();}
  TPIE_OS_OFFSET blueSize() const { return adaptor_[1]->size();}
  void getMbr(const char *input_filename, rectangle *mbr);
};

#endif // _SORTSWEEP_H
