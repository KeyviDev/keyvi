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

//
// File: parse_args.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/7/94
//
// $Id: parse_args.h,v 1.3 2003-09-11 18:45:57 jan Exp $
//
#ifndef _PARSE_ARGS_H
#define _PARSE_ARGS_H

#include "app_config.h"
#include "getopt.h"

void usage(void (*usage_app)(void) = NULL);

// Parse arguments for flags common to all test apps, as well as those
// specific to this particular app.  aso points to a string of app
// specific options, and parse_app is a pointer to a function, tpyically
// just a big switch, to handle them.
void parse_args(int argc, char **argv, const char *aso = NULL,
                void (*parse_app_opt)(char opt, char *optarg) = NULL);
                
#endif // _PARSE_ARGS_H 
