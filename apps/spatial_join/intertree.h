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

// Copyright (c) 1997 Octavian Procopiuc/Sridhar Ramaswamy/Torsten Suel
//
// File: intertree.h
// Author: Sridhar Ramaswamy/Torsten Suel
// Created: 06/28/97
// Last Modified: 06/29/97 by Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: intertree.h,v 1.3 2005-11-15 15:36:09 jan Exp $
//
// class intertree representing an interval tree.
// class internode representing a node in the interval tree.
//

#ifndef _INTERTREE_H
#define _INTERTREE_H

#include "app_config.h"
#include <ami.h>
#include "rectangle.h"

#ifdef _WIN32
#include "rand48.h"
#endif
extern "C" { long nrand48(unsigned short *);}

enum { bitsInRandom = 31 };            // aah. a magic constant. this is the
                                       // number of non-sign bits in a long.
enum { MaxLevel = 64 };                // another magic constant. This is
                                       // actually Floor(log_2(max number 
				       // of elements
                                       // that we are planning to store))


#define INVALIDID -1                   // ID for empty header and tail nodes 

// what I would really like to define here are operators min= and max=
// that work like *= or += but with minimum and maximum operator 
//
#define MINEQ(a,b)  {if ((a) > (b)) (a) = (b);}
#define MAXEQ(a,b)  {if ((a) < (b)) (a) = (b);}

// some constants to set probability of coin flips to PCO / 2^LOGPROB
//
#define PCO 1 
#define LOGPROB 2   
#define PROBM1 3      /* 2^LOGPROB - 1 */


class InterNode;

//
// structural information about the tree maintained inside each node: 
// contains forward pointers and some info on the subtree needed in Insert, 
// Search, and DeleteOld
//
typedef struct {
  coord_t xHighMax;            // maximum right boundary of intervals in subtree
  coord_t yHighMin;            // minimum expiration time of intervals in subtree
  InterNode *forward;        // pointer to next node on same level
} TreeInfo;


//
// supplies efficient coin flips (with p = 1/(2^LOGPROB)) using nrand48
//   
//
class randInfo {
  int randomsLeft;
  int randomBits;
  unsigned short xsubi[3];
public:
  inline randInfo(unsigned seed)
  {
    xsubi[0] = seed & 0xffff;
    xsubi[1] = seed >> 16;
    xsubi[2] = 0x1234;
    randomBits = nrand48(xsubi);
    randomsLeft = bitsInRandom/2;
  }
  // This sets p to be 1/(2^LOGPROB). Not easy to figure out exactly how!
  inline int randomLevel()
  {
    register int level = 0;
    register int b;
    do {
      b = randomBits&PROBM1;            // grab the last two bits of randomBits
      randomBits >>= LOGPROB;           // throw them away
      if (b < PCO) level++;             // !b holds with probability 1/4
      if (--randomsLeft == 0) {         // need a refill?
        randomBits = nrand48(xsubi);
        randomsLeft = bitsInRandom/PROBM1;   // I have 15 sets of 2 bits 
					     // apiece in a long
      }
    } while (b < PCO);                       

    return ((level > MaxLevel) ? MaxLevel : level);
  }
};



//
// classes for interval tree and its nodes 
// 
class InterTree {
  int level;
  InterNode *NIL;
  InterNode *header;
  int numElems;
  int swap;

public:
  InterTree(int swap = 0);
  void DeleteAll();                     // delete the entire interval tree
  AMI_err Insert(const rectangle &, randInfo *);   // insert new rectangle
  int Search(const rectangle &, 
	AMI_STREAM<pair_of_rectangles> *);  // search all overlapping rects
  int DeleteOld(coord_t);                 // delete all expired rectangles
  int Print();                          // guess what this does
  void Check(coord_t);
private:
  //  int num, lev;
  //  InterNode *iNode;
  pair_of_rectangles intersection;
};

class InterNode {
  friend class InterTree;
  int id;
  coord_t xLow;
  coord_t yLow;
  TreeInfo treeInfo[1];
public:
  //static int out_of_memory;
  InterNode(const rectangle &, int);
  InterNode(int, InterNode *);
  inline InterNode();
  inline void *operator new(size_t, int);
};




#endif //_INTERTREE_H
