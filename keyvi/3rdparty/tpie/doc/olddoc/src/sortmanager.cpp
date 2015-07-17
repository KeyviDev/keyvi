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

// Here is the definition of the sort management class
class SortManager {
private:
    int result;
public:
   inline int compare (const double & k1, const double & k2) {
      return ((k1 < k2)? -1 : (k1 > k2) ? 1 : 0);
   }
   inline void copy (double *key, const rectangle &record) {
      *key = record.southwest_y;
   }
};

// create a sort management object
SortManager <rectangle,double> smo;

AMI_STREAM<rectangle> instream;
AMI_STREAM<rectangle> outstream;
double dummyKey;

void f()
{
    AMI_key_sort(&instream, &outstream, dummyKey, &smo );
}
