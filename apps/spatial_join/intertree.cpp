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

// Copyright (c) 1999 Octavian Procopiuc/Sridhar Ramaswamy/Torsten Suel
//
// File: intertree.cpp
// Author: Sridhar Ramaswamy/Torsten Suel
// Created: 06/28/97
// Last Modified: 01/25/99 by Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: intertree.cpp,v 1.2 2005-11-10 10:35:57 adanner Exp $
//
// class intertree representing an interval tree.
// class internode representing a node in the interval tree.
//

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::istream;
using std::ostream;

#include "intertree.h"
#include <assert.h>
//
// creating and initializing nodes of the interval tree
//
inline void *InterNode::operator new(size_t size, int maxlev)
{
  void *addr;

  int mem = sizeof(InterNode) + maxlev * sizeof(TreeInfo);
  addr = (void *)new char[mem];

  if (addr == NULL)
    cout << "Allocation failed!" << endl;

  return (addr);
}

/* initialize new node of interval tree with a rectangle */
InterNode::InterNode(const rectangle &rect, int maxlev)
{
  id = rect.id;
  xLow = rect.xlo; 
  yLow = rect.ylo; 
  treeInfo[0].xHighMax = rect.xhi;
  treeInfo[0].yHighMin = rect.yhi;
  for (int i = 0; i <= maxlev; i++)
    treeInfo[i].forward = NULL;
}

/* initialization routine used for header node of tree */
InterNode::InterNode(int maxlev, InterNode *nilptr)
{
  id = INVALIDID;
  xLow = MINUSINFINITY;
  treeInfo[0].xHighMax = MINUSINFINITY;
  treeInfo[0].yHighMin = TP_INFINITY;
  for (int i = 0; i <= maxlev; i++)
    treeInfo[i].forward = nilptr;
}

/* initialization routine used for tail nodes of tree */
inline InterNode::InterNode()
{
  id = INVALIDID;
  xLow = TP_INFINITY;
  treeInfo[0].xHighMax = TP_INFINITY;
  treeInfo[0].yHighMin = TP_INFINITY;
  treeInfo[0].forward = NULL;
}



//
// creating, deleting, printing, and checking an interval tree 
//
InterTree::InterTree(int swapVal)
{
  level = 0;
  numElems = 0;
  swap = swapVal;
  // left = l;
  // right = r;
  NIL = new(0) InterNode();
  header = new(MaxLevel) InterNode(MaxLevel, NIL);
  
}

void InterTree::DeleteAll()
{
  InterNode *tmp1, *tmp2;
  tmp1 = header;
  while (tmp1 != NIL) {
    tmp2 = tmp1->treeInfo[0].forward;
    delete tmp1;
    tmp1 = tmp2;
  };
  delete tmp1;
  delete header;
  delete this;
}

int InterTree::Print()
{
  int i;
  int num;
  InterNode *tmp1;

  cout << "Number of elements: " << numElems << endl;
  for (i = level; i >= 0; i--)
  {
    num = -1;
    tmp1 = header;
    while (tmp1 != NIL)
    {
      cout << "ID: " << tmp1->id << " record val " << "   ";
      cout << tmp1->xLow << "   " << tmp1->treeInfo[i].xHighMax << "   ";
      cout << tmp1->yLow << "   " << tmp1->treeInfo[i].yHighMin << endl;
      tmp1 = tmp1->treeInfo[i].forward;
      num++;
    }
    cout << "Finished level:  " << i << endl;
  }
  cout << "total elements = " << num << endl;
  return num;
}

void InterTree::Check(coord_t low)
{
  int lev;
  int num;
  int foundMin, foundMax;
  coord_t xLowOld;
  InterNode *iNode, *iNode1;

  /* check higher levels for consistency */
  for (lev = level; lev > 0; lev--)
  {
    for (iNode = header->treeInfo[lev].forward, num = 0; 
         iNode != NIL; iNode = iNode->treeInfo[lev].forward)
    {
      num++;
      foundMin = foundMax = 0;
      for (iNode1 = iNode; iNode1 != iNode->treeInfo[lev].forward;
           iNode1 = iNode1->treeInfo[lev-1].forward)
      {
        if (iNode1->treeInfo[lev-1].xHighMax > iNode->treeInfo[lev].xHighMax)
          cout << "Computation of xHighMax not correct!" << endl;
        if (iNode1->treeInfo[lev-1].yHighMin < iNode->treeInfo[lev].yHighMin)
          cout << "Computation of yHighMin not correct!" << endl;
        if (iNode1->treeInfo[lev-1].xHighMax == iNode->treeInfo[lev].xHighMax)
          foundMax = 1;
        if (iNode1->treeInfo[lev-1].yHighMin == iNode->treeInfo[lev].yHighMin)
          foundMin = 1;
      }

      if (foundMax == 0)
        cout << "Computation of xHighMax not correct!" << endl;
      if (foundMin == 0)
        cout << "Computation of yHighMin not correct!" << endl;
    }
    cout << "Finished level " << lev << ":  " << num << "  nodes" << endl;
  }

  /* now check zero level for ordering */
  xLowOld = MINUSINFINITY;
  for (iNode = header->treeInfo[0].forward, num = 0; 
       iNode != NIL; iNode = iNode->treeInfo[0].forward)
  {
    num++;
    if (iNode->xLow < xLowOld)
      cout << "Ordering not correct!" << endl;
    if (iNode->treeInfo[0].yHighMin < low)
      cout << "Old element not deleted!" << endl;
        
    xLowOld = iNode->xLow;
  }

  cout << "Finished level 0:  " << num << "  nodes" << endl;
}



//
// insert a new node into the interval tree
// 
AMI_err InterTree::Insert(const rectangle &rect, randInfo *rand)
{
  InterNode *update[MaxLevel];
  InterNode *iNode, *iNode1, *iNode2;
  TreeInfo *tInfo, *tInfo1, *tInfo2;
  int lev, newLev, oldLev;


  numElems++;
  
  /* determine level of new node */
  newLev = rand->randomLevel();
  oldLev = level;
  if (newLev > level) 
  {
    if (level == MaxLevel)
      cout << "Tree has too many levels!" << endl;
    newLev = ++level;
    update[newLev] = header;
  }

#ifdef MEMORY_CHECK
  size_t sz_avail;
  MM_manager.available(&sz_avail);

  if (sz_avail <= sizeof(InterNode) + (newLev+1) * sizeof(TreeInfo))
    return AMI_ERROR_INSUFFICIENT_MAIN_MEMORY;
#endif

  /* generate new node, and copy data into it */
  iNode = (InterNode *) new(newLev) InterNode(rect, newLev);

  /* find place where node is inserted */ 
  iNode1 = header;
  for (lev = oldLev; lev >=0; lev--)
  {
    while (iNode2 = iNode1->treeInfo[lev].forward, iNode2->xLow < rect.xlo)
      iNode1 = iNode2;

    /* if still above new node, update treeInfo in top-down fashion */
    if (lev > newLev)
    {
      tInfo = &(iNode1->treeInfo[lev]);
      MAXEQ(tInfo->xHighMax, rect.xhi);
      MINEQ(tInfo->yHighMin, rect.yhi);
    }
    /* otherwise store node for later bottom-up updating */
    else
      update[lev] = iNode1;
  }

  /* now link up new node at the bottom level */
  iNode->treeInfo[0].forward = (update[0])->treeInfo[0].forward;
  (update[0])->treeInfo[0].forward = iNode;
 
  /* now do the remaining updates in bottom-up fashion */
  for (lev = 1; lev <= newLev; lev++)
  {
    iNode1 = update[lev];
    tInfo = &(iNode->treeInfo[lev]);
    tInfo1 = &(iNode1->treeInfo[lev]);

    /* link up new node at this level */
    tInfo->forward = tInfo1->forward;
    tInfo1->forward = iNode;

    /* compute treeInfo of new node from next lower level */
    tInfo->xHighMax = MINUSINFINITY;
    tInfo->yHighMin = TP_INFINITY;
    for (iNode2 = iNode; iNode2 != tInfo->forward; iNode2 = tInfo2->forward)
    {
      tInfo2 = &(iNode2->treeInfo[lev-1]);
      MAXEQ(tInfo->xHighMax, tInfo2->xHighMax);
      MINEQ(tInfo->yHighMin, tInfo2->yHighMin);
    }

    /* update treeInfo of predecessor of new node from next lower level */
    tInfo1->xHighMax = MINUSINFINITY;
    tInfo1->yHighMin = TP_INFINITY;
    for (iNode2 = iNode1; iNode2 != tInfo1->forward; iNode2 = tInfo2->forward)
    {
      tInfo2 = &(iNode2->treeInfo[lev-1]);
      MAXEQ(tInfo1->xHighMax, tInfo2->xHighMax);
      MINEQ(tInfo1->yHighMin, tInfo2->yHighMin);
    }
  }
  return AMI_ERROR_NO_ERROR;
}
    


//
// delete all intervals in the tree that have yHigh < time. 
//
// Can be changed to yHigh<=time by changing "<time" to "<=time" (2 occurrences).
// Returns number of deleted intervals. Uses several stacks for a non-
// recursive tree traversal. Same basic structure and traversal technique 
// as the Search procedure below.
//
int InterTree:: DeleteOld(coord_t time)
{
  InterNode *nodeStack[MaxLevel];  // stack to store last visited nodes in higher
  InterNode *endStack[MaxLevel];   // levels and corresponding end marker nodes
  InterNode *iNode = header;   // pointer to currently visited node 
  InterNode *endNode = NIL;    // pointer to end of current horizontal traversal
  InterNode *nextNode, *tmpNode;
  TreeInfo *tInfo;
  int visitSubtree;
  int num = 0;                 // number of intervals deleted 
  int lev = level;             // level of current horizontal list traversal 

  if (!numElems)
    return 0;

  //
  // Search terminates once lev is increased to level+1 in inner while loop.
  // In this case we have traversed the top layer of the list up to the end.
  //

  while (lev <= level)
  {
    /* check if we have to visit subtree corresponding to this node */
    visitSubtree = (iNode->treeInfo[lev].yHighMin < time);

    /* find next node on this level, skipping to-be-deleted nodes */
    nextNode = iNode->treeInfo[lev].forward;
    while (nextNode->treeInfo[0].yHighMin < time)
    {
      /* if to-be-deleted node encountered, we have to visit subtree */ 
      visitSubtree = 1;
      tmpNode = nextNode;
      nextNode = nextNode->treeInfo[lev].forward;

      /* also, change pointers to skip this node and, if at level 0, delete it */
      iNode->treeInfo[lev].forward = nextNode;
      if (lev == 0)
      {
        numElems--;
        delete tmpNode;
        num++;
      }
    }

    /* if subtree has node to be deleted, put stuff on stack and enter subtree */
    if (visitSubtree && (lev > 0))
    {
      nodeStack[lev] = iNode;
      endStack[lev] = endNode;

    /* take next not-to-be-deleted node as end of lower-level horizontal traversal */
      endNode = nextNode;

      /* reset treeInfo fields for recomputation, and go down into subtree */
      iNode->treeInfo[lev].xHighMax = MINUSINFINITY;
      iNode->treeInfo[lev].yHighMin = TP_INFINITY;
      lev--;
    } 
    else 
    {
      /* if not in top level, update treeInfo of parent of this node */
      if (lev < level)
      {
        tInfo = &((nodeStack[lev+1])->treeInfo[lev+1]);
        MAXEQ(tInfo->xHighMax, iNode->treeInfo[lev].xHighMax); 
        MINEQ(tInfo->yHighMin, iNode->treeInfo[lev].yHighMin); 
      }

      /* try to go to next not-to-be-deleted node on the right */ 
      iNode = iNode->treeInfo[lev].forward;
      while ((iNode == endNode) && (lev <= level))
      {
        lev++;

        /* if not in top level, get things from stack */
        if (lev <= level)
        {
          iNode = nodeStack[lev];
          endNode = endStack[lev];
        }

        if (lev < level)
        {
          /* update treeInfo of parent of this node */
          tInfo = &((nodeStack[lev+1])->treeInfo[lev+1]);
          MAXEQ(tInfo->xHighMax, iNode->treeInfo[lev].xHighMax); 
          MINEQ(tInfo->yHighMin, iNode->treeInfo[lev].yHighMin); 
        }

        /* move one node forward */
        if (lev <= level)
          iNode = iNode->treeInfo[lev].forward;
      }
    }   
  }

  while (header->treeInfo[level].forward == NIL)
    level--;
    
  return(num);
}



//
// search for all intervals that overlap a given interval
//
// Returns number of intervals found. Uses two stacks for
// a non-recursive tree traversal. Kind'o tricky.
//
int InterTree::Search(const rectangle &rect, 
		      AMI_STREAM<pair_of_rectangles> *outstr) 
{
  InterNode *nodeStack[MaxLevel];  // stack to store last visited nodes in higher
  InterNode *endStack[MaxLevel];   // levels and corresponding end marker nodes
  InterNode *iNode = header;       // pointer to currently visited node 
  InterNode *endNode = NIL;    // pointer to end of current horizontal traversal
  int num = 0;                 // number of intersections found 
  int lev = level;             // level of current horizontal list traversal 

  if (!numElems) return 0;
  //
  // Search terminates once lev is increased to level+1 in inner while loopi,
  // or if we encounter a node whose left boundary is to the right of
  // our rectangle (see first if statement below). In the first case we have 
  // traversed the topmost layer of the list up to the end, while in the later
  // case we can stop the search because things are sorted by xLow.  
  //

  while (lev <= level)
  {
    /* if node is right of rectangle, no further intersections to be found */  
    if (iNode->xLow > rect.xhi)
      return(num);
    
    /* if subtree could have intersection, put stuff on stack and enter subtree */
    if ((lev > 0) && (iNode->treeInfo[lev].xHighMax >= rect.xlo))
    {
      nodeStack[lev] = iNode;
      endStack[lev] = endNode;
      endNode = iNode->treeInfo[lev].forward;
      lev--;
    }
    else
    {
      /* if this node represents an intersecting rectangle, report it */
      if (iNode->treeInfo[lev].xHighMax >= rect.xlo)
      {
        /* check rectangle in tree hasn't expired yet */
        if ((iNode->treeInfo[lev].yHighMin >= rect.ylo)
	    /* && (iNode->xLow >= left || rect.xlo >= left)*/) {
	  if (swap) {
	    intersection.first = iNode->id;
	    intersection.second = rect.id;
	  } else {
	    intersection.first = rect.id;
	    intersection.second = iNode->id;
	  }	    

	  outstr->write_item(intersection);
          num++;
        }
      }
    
      /* try to go to the next node to the right */
      iNode = iNode->treeInfo[lev].forward;

      // if this is the end of current horizontal list traversal, go to next 
      // higher levels and try to go right, until you can do it or lev=level+1
      // 
      while ((iNode == endNode) && (lev <= level))
      {
        lev++;
        if (lev <= level)
        { 
          iNode = nodeStack[lev];
          endNode = endStack[lev];
          iNode = iNode->treeInfo[lev].forward;
        }
      }
    }   
  }

  return(num);
}
