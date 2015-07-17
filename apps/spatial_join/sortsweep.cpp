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
// File:         sortsweep.cpp
// Author:       Octavian Procopiuc <tavi@cs.duke.edu>
// Created:      01/24/99
// Description:  Internal sweep class.
//
// $Id: sortsweep.cpp,v 1.1 2003-11-21 17:01:09 tavi Exp $
//
#include <fstream>
#include <iostream>
using std::ifstream;
using std::istream;
using std::ostream;
using std::endl;
using std::cerr;
#include <string.h>
#include "app_config.h"
#include "sortsweep.h"
//#include "intertree.h"
#include "striped.h"

sort_sweep::sort_sweep(const char* red_filename, const char* blue_filename,
		     AMI_STREAM<pair_of_rectangles> *outStream): 
  outStream_(outStream), intersection_count(-1), counter(0) {

  adaptor_[0] = new SortingAdaptor(red_filename);
  adaptor_[1] = new SortingAdaptor(blue_filename);

  rectangle buf_mbr;
  getMbr(red_filename, &buf_mbr);
  getMbr(blue_filename, &mbr_);
  mbr_.extend(buf_mbr);
#if (COORD_T==INT)
  mbr_.xhi += 1;
  mbr_.xlo -= 1;
#endif
};

sort_sweep::~sort_sweep() {
  delete adaptor_[0];
  delete adaptor_[1];
};

AMI_err sort_sweep::run() {

  int b1, b2, i, count[2][MAX_STRIPS];
  chunk *c[2][MAX_STRIPS], *cEnd[2][MAX_STRIPS], *ch, *cc;
  rectangle *r, *rr;
  coord_t lb[MAX_STRIPS];
  double w;
  pair_of_rectangles pair;

  int nStrips = MAX_STRIPS;

  for (i = 0; i < nStrips; i++)
  {
    count[0][i] = count[1][i] = 0;
    //    c[0][i] = (chunk *) malloc(sizeof(chunk));
    c[0][i] = new chunk;
    //    c[1][i] = (chunk *) malloc(sizeof(chunk));
    c[1][i] = new chunk;
    c[0][i]->next = c[1][i]->next = NULL; 
    c[0][i]->num = c[1][i]->num = 0;
    cEnd[0][i] = c[0][i];
    cEnd[1][i] = c[1][i];
  }
  /* store inverse of strip width, left limit, and left boundaries of strips */
  w = ((double)(nStrips)) / (mbr_.xhi - mbr_.xlo);
  for (i = 0; i < nStrips; i++)
    lb[i] = (int) (mbr_.xlo + (double)(i) / w);


  intersection_count = 0;

  // the big loop. One element is read in each iteration.
  while (true) {

    if (adaptor_[0]->empty()) 
      if (adaptor_[1]->empty()) 
	break; // the only exit from the while loop.
      else
	crect = 1;
    else // ie, !adaptor_[0].empty()
      if (adaptor_[1]->empty())
	crect = 0;
      else
	crect = (adaptor_[0]->getMinY() <= adaptor_[1]->getMinY())? 0 : 1;

    // just so that I don't compute it every time.
    //    ncrect = (crect + 1) % 2;
    ncrect = (crect ^ 1) & 1;
    
    // read the next item from the adaptor
    if ((err = adaptor_[crect]->read_item(&prect)) != AMI_ERROR_NO_ERROR)
      return err;

    /* determine which chunks rectangle falls into */
    b1 = (int)((prect->xlo - mbr_.xlo) * w);
    b2 = (int)((prect->xhi - mbr_.xlo) * w);
    
    /* loop over all chunks touched by rectangle */
    for (i = b1; i <= b2; i++) {
        /* take next rectangle from first set, and insert it into chunk */ 
        r = cEnd[crect][i]->rects + (cEnd[crect][i]->num++);
	*r = *prect;
        if (cEnd[crect][i]->num == C_SIZE) {
	  //cEnd[crect][i]->next = (chunk *) malloc(sizeof(chunk));
	  cEnd[crect][i]->next = new chunk;
	  cEnd[crect][i] = cEnd[crect][i]->next;
	  cEnd[crect][i]->next = NULL; 
	  cEnd[crect][i]->num = 0;
	}
	
        /* check if we should delete old elements from other list */
        if (++count[ncrect][i] == DELETE_FREQUENCY) {
          count[ncrect][i] = 0;
	  
          /* compact all non-expired rectangles to beginning of list */
          cc = c[ncrect][i];
          rr = cc->rects;
          for (ch = c[ncrect][i]; ch != NULL; ch = ch->next)
            for (r = ch->rects; r < ch->rects + ch->num; r++) {
              if (r->yhi >= prect->ylo) {
		*rr = *r;
                if (++rr == cc->rects + C_SIZE) {
                  cc = cc->next;
                  rr = cc->rects;
                }
              }
            }
	  
          /* update information */
          cEnd[ncrect][i] = cc;
          cEnd[ncrect][i]->num = rr - cc->rects;
          cc = cc->next;
          cEnd[ncrect][i]->next = NULL;
	  
          /* delete all empty list elements */
          while (cc != NULL) {
            ch = cc;
            cc = cc->next;
            //free (c); 
	    delete ch;
          }
        }
	
        /* search other tree for intersecting elements */
        for (ch = c[ncrect][i]; ch != NULL; ch = ch->next)
          for (r = ch->rects; r < ch->rects + ch->num; r++)
            if ((r->xlo <= prect->xhi) && (r->xhi >= prect->xlo) &&
                (r->yhi >= prect->ylo) &&
                ((r->xlo >= lb[i]) || (prect->xlo >= lb[i]))) {
	      if (crect == 0) {
		pair.first = prect->id;
		pair.second = r->id;
	      } else {
		pair.first = r->id;
		pair.second = prect->id;
	      }		
	      outStream_->write_item(pair);
	      intersection_count++;
            }
    } // end of for i.

#ifdef PRINT_STATISTICS
    replication += b2 - b1;
#endif
  } // end of while true.

/*
#ifdef STRIPED_SWEEP
  // cleanup
  for (i = 0; i < nStrips; i++)
  {
    while (c[0][i] != NULL)
    {
      ch = c[0][i];
      c[0][i] = c[0][i]->next;
      delete ch; 
    }
    while (c[1][i] != NULL)
    {
      ch = c[1][i];
      c[1][i] = c[1][i]->next;
      delete ch; 
    }
  }
#endif STRIPED_SWEEP
*/
  return AMI_ERROR_NO_ERROR;
};

void sort_sweep::getMbr(const char *input_filename, rectangle *mbr) {
  // Add suffix ".mbr" to the input file name.
  char *mbr_filename = new char[strlen(input_filename)+5];
  strcpy(mbr_filename, input_filename);
  mbr_filename = strcat(mbr_filename, ".mbr");
  // Read the mbr.
  ifstream *mbr_file_stream = new ifstream(mbr_filename);
  if (!(*mbr_file_stream))
    cerr << "Error: couldn't open " << mbr_filename << endl;
  else 
    mbr_file_stream->read((char *) mbr, sizeof(rectangle));

  delete mbr_file_stream;
  delete [] mbr_filename;
};
