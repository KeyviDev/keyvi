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

class compare_re_class {
public:
    int compare ( const complex &c1, const complex &c2 ) {
        return (c1.re() < c2.re()) ? -1 :
               ((c1.re() > c2.re()) ? 1 : 0);
    };
};

class compare_im_class {
public:
    int compare ( const complex &c1, const complex &c2 ) {
        return (c1.im() < c2.im()) ? -1 :
               ((c1.im() > c2.im()) ? 1 : 0);
    };
};

AMI_STREAM<complex> instream;
AMI_STREAM<complex> outstream_re;
AMI_STREAM<complex> outstream_im;

compare_re_class compare_re
compare_im_class compare_im

void f()
{
    AMI_sort(&instream, &outstream_re, &compare_re);
    AMI_sort(&instream, &outstream_im, &compare_im);
}
