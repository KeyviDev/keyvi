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

#ifndef _TPIE_AMI_STREAM_ARITH_H
#define _TPIE_AMI_STREAM_ARITH_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
// Get the definition of the scan_object class.
#include <tpie/scan.h>

namespace tpie {

    namespace ami {
	
#define SCAN_OPERATOR_DECLARATION(NAME,OP)				\
									\
	template<class T> class scan_ ## NAME : scan_object {		\
	public:								\
	    err initialize(void);					\
	    err operate(const T &op1, const T &op2, SCAN_FLAG *sfin,	\
			T *res, SCAN_FLAG *sfout);			\
	};								\
									\
		template<class T>					\
		err scan_ ## NAME<T>::initialize(void)			\
		{							\
		    return NO_ERROR;					\
		}							\
									\
									\
		template<class T>					\
		err scan_ ## NAME<T>::operate(const T &op1, const T &op2, \
					      SCAN_FLAG *sfin,		\
					      T *res, SCAN_FLAG *sfout)	\
		{							\
		    if ((*sfout = (sfin[0] && sfin[1]))) {		\
			*res = op1 OP op2;				\
			return SCAN_CONTINUE;				\
		    } else {						\
			return SCAN_DONE;				\
		    }							\
		}
	
	SCAN_OPERATOR_DECLARATION(add,+)
	    SCAN_OPERATOR_DECLARATION(sub,-)
	    SCAN_OPERATOR_DECLARATION(mult,*)
	    SCAN_OPERATOR_DECLARATION(div,/)
	    
    }  //  ami namespace

}  // tpie namespace


namespace tpie {

    namespace ami {

#define SCAN_SCALAR_OPERATOR_DECLARATION(NAME,OP)			\
									\
	template<class T> class scan_scalar_ ## NAME : scan_object {	\
	private:							\
	    T scalar;							\
	public:								\
	    scan_scalar_ ## NAME(const T &s);			  	\
	    virtual ~scan_scalar_ ## NAME(void);			\
	    err initialize(void);					\
	    err operate(const T &op, SCAN_FLAG *sfin,			\
			T *res, SCAN_FLAG *sfout);			\
	};								\
									\
									\
		template<class T>					\
		scan_scalar_ ## NAME<T>::				\
		scan_scalar_ ## NAME(const T &s) :			\
		    scalar(s)						\
		{							\
		}							\
									\
									\
		template<class T>					\
		scan_scalar_ ## NAME<T>::~scan_scalar_ ## NAME()	\
		{							\
		}							\
									\
									\
		template<class T>					\
		err scan_scalar_ ## NAME<T>::initialize(void)		\
		{							\
		    return NO_ERROR;					\
		}							\
									\
									\
		template<class T>					\
		err scan_scalar_ ## NAME<T>::operate(const T &op,	\
						     SCAN_FLAG *sfin,	\
						     T *res, SCAN_FLAG *sfout) \
		{							\
		    if ((*sfout = *sfin)) {				\
			*res = op OP scalar;				\
			return SCAN_CONTINUE;				\
		    } else {						\
			return SCAN_DONE;				\
		    }							\
		}
	
	
	SCAN_SCALAR_OPERATOR_DECLARATION(add,+)
	    SCAN_SCALAR_OPERATOR_DECLARATION(sub,-)
	    SCAN_SCALAR_OPERATOR_DECLARATION(mult,*)
	    SCAN_SCALAR_OPERATOR_DECLARATION(div,/)

	    }  //  ami namespace

}  //  tpie namespace

#endif // _AMI_STREAM_ARITH_H 
