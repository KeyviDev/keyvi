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

///////////////////////////////////////////////////////////////////////////
/// \file scan.h 
/// The function AMI_scan() reads zero, one or multiple input streams (up
/// to four), each potentially of a different type, and writes zero, one
/// or multiple output streams (up to four), each potentially of a
/// different type. 
/// \sa tpie::ami::scan_object
///////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_AMI_SCAN_H
#define _TPIE_AMI_SCAN_H

#include <tpie/err.h>
#include <tpie/stream.h>

namespace tpie {
    
    namespace ami {
	
	typedef int SCAN_FLAG;

	///////////////////////////////////////////////////////////////////////////
	/// The base class for scan objects. 
	/// \anchor scan_management_objects
	/// \par Scan Management Objects
	/// A scan management object class must inherit from
	/// scan_object ass follows:
	/// template<class T1, class T2,..., class U1, class U2,...> class ST:
	/// public scan_object;
	/// In addition, it must provide two member functions for
	/// scan() to call: scan_object#initialize() and
	/// operate().
	///    \par operate() 
	/// Most of the work of a scan is
	/// typically done in the scan management object's operate() member function:
	///
	/// err operate(const T1 &in1, const T2 &in2,..., SCAN_FLAG
	/// *sfin, U1 *out1, U2 *out2,..., SCAN_FLAG *sfout);
	/// 
	/// One or more input objects or one or more output parameters must be
	/// specified. These must correspond in number and type to the streams
	/// passed to the polymorph of AMI_scan() with which this scan
	/// management object is to be used.
	/// 
	/// If present, the inputs *in1, ... are application data
	/// items of type T1, and sfin points to an array
	/// of flags, one for each input. On entry to operate(),
	/// flags that are set (non-zero) indicate that the corresponding inputs
	/// contain data. If on exit from operate(), the input flags
	/// are left untouched, AMI_scan() assumes that the
	/// corresponding inputs were processed. If one or more input flags are
	/// cleared (set to zero) then AMI_scan() assumes that the
	/// corresponding inputs were not processed and should be presented again
	/// on the next call to operate(). This permits out of step
	/// scanning \anchor scanning_out_of_step, as illustrated in Section  
	/// "Out of step scanning" in the manual.
	/// \internal \todo Do not refer to the manual in the docs
	/// 
	/// If present, the outputs *out1, ... are application data
	/// items of type U1, and sfout points to an array
	/// of flags, one for each output. On exit from operate(), the
	/// outputs should contain any objects to be written to the output
	/// streams, and the output flags must be set to indicate to
	/// AMI_scan() which outputs are valid and should be written
	/// to the output streams.
	/// 
	/// The return value of operate() will normally be either \ref SCAN_CONTINUE, or
	/// \ref SCAN_DONE. Note that operate() is permitted to return
	/// SCAN_CONTINUE even when the input flags indicate
	/// that there is no more input to be processed. This is useful if
	/// the scan management object maintains some internal state that must
	/// be written out after all input has been processed.
	/// \sa scan.h, scan_management_objects
  ///////////////////////////////////////////////////////////////////////////
	class scan_object {

	public:
	  ///////////////////////////////////////////////////////////////////////////
	  /// Semantics for subclass implementations: Initializes a scan management object to prepare 
	  /// it for a scan.
	  /// This member function is called once by
	  /// each call to scan() in order to initialize
	  /// the scan management object before any data processing
	  /// takes place. This function should return
	  /// \ref NO_ERROR if successful, or an appropriate error otherwise. See
	  /// Section \ref sec:ami-errors for a list of error codes.
	  ///////////////////////////////////////////////////////////////////////////
	  virtual err initialize(void) = 0;
	  virtual ~scan_object() {};
	};

    }  //  ami namespace

}  // tpie namespace

// BEGIN MECHANICALLY GENERATED CODE.

#ifndef _TPIE_AMI_SCAN_MAC_H
#define _TPIE_AMI_SCAN_MAC_H

namespace tpie {

    namespace ami {

// Macros for defining parameters to AMI_scan()
#define __SPARM_BASE(T,io,n) stream< T ## n > *io ## n
#define __SPARM_1(T,io) __SPARM_BASE(T,io,1)  
#define __SPARM_2(T,io) __SPARM_1(T,io), __SPARM_BASE(T,io,2)
#define __SPARM_3(T,io) __SPARM_2(T,io), __SPARM_BASE(T,io,3)
#define __SPARM_4(T,io) __SPARM_3(T,io), __SPARM_BASE(T,io,4)

// Macros for defining types in a template for AMI_scan()
#define __STEMP_BASE(T,n) class T ## n
#define __STEMP_1(T) __STEMP_BASE(T,1)  
#define __STEMP_2(T) __STEMP_1(T), __STEMP_BASE(T,2)
#define __STEMP_3(T) __STEMP_2(T), __STEMP_BASE(T,3)
#define __STEMP_4(T) __STEMP_3(T), __STEMP_BASE(T,4)

// Temporary space used within AMI_scan
#define __STS_BASE(T,t,n) T ## n t ## n 
#define __STSPACE_1(T,t) __STS_BASE(T,t,1)
#define __STSPACE_2(T,t) __STSPACE_1(T,t) ; __STS_BASE(T,t,2) 
#define __STSPACE_3(T,t) __STSPACE_2(T,t) ; __STS_BASE(T,t,3) 
#define __STSPACE_4(T,t) __STSPACE_3(T,t) ; __STS_BASE(T,t,4) 

// An array of flags.
#define __FSPACE(f,n) SCAN_FLAG f[n]


// Check stream validity.
#define __CHK_BASE(T,n) {                                               \
    if (T ## n == NULL || T ## n -> status() != STREAM_STATUS_VALID) {\
        return GENERIC_ERROR;                                 \
    }                                                                   \
}

#define __CHKSTR_1(T) __CHK_BASE(T,1)
#define __CHKSTR_2(T) __CHKSTR_1(T) __CHK_BASE(T,2)
#define __CHKSTR_3(T) __CHKSTR_2(T) __CHK_BASE(T,3)
#define __CHKSTR_4(T) __CHKSTR_3(T) __CHK_BASE(T,4)


// Rewind the input streams prior to performing the scan.
#define __REW_BASE(T,n) {						\
    if ((_ami_err_ = T ## n -> seek(0)) != NO_ERROR) {	\
        return _ami_err_;						\
    }									\
}

#define __REWIND_1(T) __REW_BASE(T,1)
#define __REWIND_2(T) __REWIND_1(T) __REW_BASE(T,2)
#define __REWIND_3(T) __REWIND_2(T) __REW_BASE(T,3)
#define __REWIND_4(T) __REWIND_3(T) __REW_BASE(T,4)


// Set the input flags to true before entering the do loop so that the
// initial values will be read.
#define __SET_IF_BASE(f,n) f[n-1] = 1

#define __SET_IF_1(f) __SET_IF_BASE(f,1)
#define __SET_IF_2(f) __SET_IF_1(f); __SET_IF_BASE(f,2)
#define __SET_IF_3(f) __SET_IF_2(f); __SET_IF_BASE(f,3)
#define __SET_IF_4(f) __SET_IF_3(f); __SET_IF_BASE(f,4)

// If the flag is set, then read inputs into temporary space.  Set the
// flag based on whether the read was succesful or not.  If it was
// unsuccessful for any reason other than EOS, then break out of the
// scan loop.  If the flag is not currently set, then either the scan
// management object did not take the last input or the last time we
// tried to read from this file we failed.  If we read successfully
// last time, then reset the flag.
#define __STSR_BASE(t,ts,f,g,e,n)					    \
if (f[n-1]) {								    \
    if (!(f[n-1] = g[n-1] =						    \
          ((e = ts ## n->read_item(&t ## n)) == NO_ERROR))) {	    \
        if (e != END_OF_STREAM) {				    \
            break;							    \
        }								    \
    }									    \
} else {								    \
    f[n-1] = g[n-1];							    \
}

#define __STS_READ_1(t,ts,f,g,e) __STSR_BASE(t,ts,f,g,e,1) 
#define __STS_READ_2(t,ts,f,g,e) __STS_READ_1(t,ts,f,g,e)		    \
        __STSR_BASE(t,ts,f,g,e,2)
#define __STS_READ_3(t,ts,f,g,e) __STS_READ_2(t,ts,f,g,e)		    \
        __STSR_BASE(t,ts,f,g,e,3)
#define __STS_READ_4(t,ts,f,g,e) __STS_READ_3(t,ts,f,g,e)		    \
        __STSR_BASE(t,ts,f,g,e,4)

// Write outputs.  Only write if the flag is set.  If there is an
// error during the write, then break out of the scan loop.
#define __STSW_BASE(u,us,f,e,n)						    \
if (f[n-1] && (e = us ## n -> write_item(u ## n)) != NO_ERROR) {  \
    break;								    \
}

#define __STS_WRITE_1(u,us,f,e) __STSW_BASE(u,us,f,e,1)
#define __STS_WRITE_2(u,us,f,e) __STS_WRITE_1(u,us,f,e) __STSW_BASE(u,us,f,e,2)
#define __STS_WRITE_3(u,us,f,e) __STS_WRITE_2(u,us,f,e) __STSW_BASE(u,us,f,e,3)
#define __STS_WRITE_4(u,us,f,e) __STS_WRITE_3(u,us,f,e) __STSW_BASE(u,us,f,e,4)


// Arguments to the operate() call
#define __SCA_BASE(t,n) t ## n
#define __SCALL_ARGS_1(t) __SCA_BASE(t,1)
#define __SCALL_ARGS_2(t) __SCALL_ARGS_1(t), __SCA_BASE(t,2) 
#define __SCALL_ARGS_3(t) __SCALL_ARGS_2(t), __SCA_BASE(t,3) 
#define __SCALL_ARGS_4(t) __SCALL_ARGS_3(t), __SCA_BASE(t,4) 

// Operate on the inputs to produce the outputs.
#define __SCALL_BASE(t,nt,if,sop,u,nu,of) \
    sop->operate(__SCALL_ARGS_ ## nt (*t), if, __SCALL_ARGS_ ## nu (&u), of)

#define __SCALL_OP_1_1(t,if,sop,u,of) __SCALL_BASE(t,1,if,sop,u,1,of)
#define __SCALL_OP_1_2(t,if,sop,u,of) __SCALL_BASE(t,1,if,sop,u,2,of)
#define __SCALL_OP_1_3(t,if,sop,u,of) __SCALL_BASE(t,1,if,sop,u,3,of)
#define __SCALL_OP_1_4(t,if,sop,u,of) __SCALL_BASE(t,1,if,sop,u,4,of)

#define __SCALL_OP_2_1(t,if,sop,u,of) __SCALL_BASE(t,2,if,sop,u,1,of)
#define __SCALL_OP_2_2(t,if,sop,u,of) __SCALL_BASE(t,2,if,sop,u,2,of)
#define __SCALL_OP_2_3(t,if,sop,u,of) __SCALL_BASE(t,2,if,sop,u,3,of)
#define __SCALL_OP_2_4(t,if,sop,u,of) __SCALL_BASE(t,2,if,sop,u,4,of)

#define __SCALL_OP_3_1(t,if,sop,u,of) __SCALL_BASE(t,3,if,sop,u,1,of)
#define __SCALL_OP_3_2(t,if,sop,u,of) __SCALL_BASE(t,3,if,sop,u,2,of)
#define __SCALL_OP_3_3(t,if,sop,u,of) __SCALL_BASE(t,3,if,sop,u,3,of)
#define __SCALL_OP_3_4(t,if,sop,u,of) __SCALL_BASE(t,3,if,sop,u,4,of)

#define __SCALL_OP_4_1(t,if,sop,u,of) __SCALL_BASE(t,4,if,sop,u,1,of)
#define __SCALL_OP_4_2(t,if,sop,u,of) __SCALL_BASE(t,4,if,sop,u,2,of)
#define __SCALL_OP_4_3(t,if,sop,u,of) __SCALL_BASE(t,4,if,sop,u,3,of)
#define __SCALL_OP_4_4(t,if,sop,u,of) __SCALL_BASE(t,4,if,sop,u,4,of)

// Handle the no input case.
#define __SCALL_BASE_O(sop,u,nu,of) \
    sop->operate(__SCALL_ARGS_ ## nu (&u), of)

#define __SCALL_OP_O_1(sop,u,of) __SCALL_BASE_O(sop,u,1,of)
#define __SCALL_OP_O_2(sop,u,of) __SCALL_BASE_O(sop,u,2,of)
#define __SCALL_OP_O_3(sop,u,of) __SCALL_BASE_O(sop,u,3,of)
#define __SCALL_OP_O_4(sop,u,of) __SCALL_BASE_O(sop,u,4,of)

// Handle the no output case.
#define __SCALL_BASE_I(t,nt,if,sop) \
    sop->operate(__SCALL_ARGS_ ## nt (*t), if)

#define __SCALL_OP_I_1(t,if,sop) __SCALL_BASE_I(t,1,if,sop)
#define __SCALL_OP_I_2(t,if,sop) __SCALL_BASE_I(t,2,if,sop)
#define __SCALL_OP_I_3(t,if,sop) __SCALL_BASE_I(t,3,if,sop)
#define __SCALL_OP_I_4(t,if,sop) __SCALL_BASE_I(t,4,if,sop)


// The template for the whole scan(), with inputs and outputs.
#define __STEMPLATE(in_arity, out_arity)				\
	template< __STEMP_ ## in_arity (T), class SC, __STEMP_ ## out_arity (U) > \
	err scan( __SPARM_ ## in_arity (T,_ts_),			\
                  SC *soper, __SPARM_ ## out_arity (U,_us_))		\
	{								\
	    __STSPACE_ ## in_arity (T,*_t_);				\
	    __STSPACE_ ## out_arity (U,_u_);				\
	    								\
	    __FSPACE(_if_,in_arity);					\
	    __FSPACE(_lif_,in_arity);					\
	    __FSPACE(_of_,out_arity);					\
	    								\
	    err _op_err_ = NO_ERROR, _ami_err_ =NO_ERROR;		\
	    								\
	    __CHKSTR_ ## in_arity (_ts_)				\
		__CHKSTR_ ## out_arity (_us_)				\
		__REWIND_ ## in_arity (_ts_)				\
		soper->initialize();					\
	    								\
	    __SET_IF_ ## in_arity (_if_);				\
	    								\
	    do {							\
	    								\
		__STS_READ_ ## in_arity (_t_,_ts_,_if_,_lif_,_ami_err_)	\
		    							\
		    _op_err_ = __SCALL_OP_ ## in_arity ## _ ##		\
		    out_arity(_t_,_if_,soper,_u_,_of_);			\
									\
		__STS_WRITE_ ## out_arity(_u_,_us_,_of_,_ami_err_)	\
		    							\
		    } while (_op_err_ == SCAN_CONTINUE);		\
	    								\
	    if ((_ami_err_ != NO_ERROR) &&				\
		(_ami_err_ != END_OF_STREAM)) {				\
		return _ami_err_;					\
	    }								\
	    								\
	    return NO_ERROR;						\
	}

// The template for the whole AMI_scan(), with no inputs.  This is
// based on __STEMPLATE_() and could be merged into one big macro at
// the expense of having to define multiple versions of __STEMP_N()
// and __SPARM_N() to handle the case N = 0.
#define __STEMPLATE_O(out_arity)					\
	template< class SC, __STEMP_ ## out_arity (U) >			\
	err scan( SC *soper, __SPARM_ ## out_arity (U,_us_))		\
	{								\
	    __STSPACE_ ## out_arity (U,_u_);				\
	    								\
	    __FSPACE(_of_,out_arity);					\
	    								\
	    err _op_err_ = NO_ERROR, _ami_err_ = NO_ERROR;		\
	    								\
	    __CHKSTR_ ## out_arity (_us_)				\
		soper->initialize();					\
	    								\
	    do {							\
	    								\
		_op_err_ = __SCALL_OP_O_ ## out_arity(soper,_u_,_of_);	\
									\
		__STS_WRITE_ ## out_arity(_u_,_us_,_of_,_ami_err_)	\
		    							\
		    } while (_op_err_ == SCAN_CONTINUE);		\
	    								\
	    if ((_ami_err_ != NO_ERROR) &&				\
		(_ami_err_ != END_OF_STREAM)) {				\
		return _ami_err_;					\
	    }								\
	    								\
	    return NO_ERROR;						\
	}

// The template for the whole AMI_scan(), with no outputs.
#define __STEMPLATE_I(in_arity)						\
	template< __STEMP_ ## in_arity (T), class SC >			\
	err scan( __SPARM_ ## in_arity (T,_ts_), SC *soper)		\
	{								\
	    __STSPACE_ ## in_arity (T,*_t_);				\
	    								\
	    __FSPACE(_if_,in_arity);					\
	    __FSPACE(_lif_,in_arity);					\
	    								\
	    err _op_err_ = NO_ERROR, _ami_err_ = NO_ERROR;		\
	    								\
	    __CHKSTR_ ## in_arity (_ts_)				\
		__REWIND_ ## in_arity (_ts_);				\
	    								\
	    soper->initialize();					\
	    								\
	    __SET_IF_ ## in_arity (_if_);				\
	    								\
	    do {							\
									\
		__STS_READ_ ## in_arity (_t_,_ts_,_if_,_lif_,_ami_err_)	\
		    							\
		    _op_err_ = __SCALL_OP_I_ ## in_arity (_t_,_if_,soper); \
									\
	    } while (_op_err_ == SCAN_CONTINUE);			\
	    								\
	    if ((_ami_err_ != NO_ERROR) &&				\
		(_ami_err_ != END_OF_STREAM)) {				\
		return _ami_err_;					\
	    }								\
	    								\
	    return NO_ERROR;						\
	}


// Finally, the templates themsleves.

#ifdef _WIN32
		// Suppress warning 4706 (assignment in if-clause).
#pragma warning(disable : 4706)
#endif

__STEMPLATE(1,1); __STEMPLATE(1,2); __STEMPLATE(1,3); __STEMPLATE(1,4);
__STEMPLATE(2,1); __STEMPLATE(2,2); __STEMPLATE(2,3); __STEMPLATE(2,4);
__STEMPLATE(3,1); __STEMPLATE(3,2); __STEMPLATE(3,3); __STEMPLATE(3,4);
__STEMPLATE(4,1); __STEMPLATE(4,2); __STEMPLATE(4,3); __STEMPLATE(4,4);

__STEMPLATE_O(1); __STEMPLATE_O(2); __STEMPLATE_O(3); __STEMPLATE_O(4);

__STEMPLATE_I(1); __STEMPLATE_I(2); __STEMPLATE_I(3); __STEMPLATE_I(4);

#ifdef _WIN32
		// Reset to default state.
#pragma warning(default : 4706)
#endif

    }  //  ami namespace

}  //  tpie namespace


#endif // _TPIE_AMI_SCAN_MAC_H 

// END MECHANICALLY GENERATED CODE.

// The following Id string applies to the tail used in the processs of
// generating this file.  The tail, stored in ami_scan.h.tail, may be
// edited if necessary.
//
// $Id: ami_scan.h.tail,v 1.3 2002-01-14 16:02:43 tavi Exp $

namespace tpie {

    namespace ami {
	
  ///////////////////////////////////////////////////////////////////////////
  /// A class template for copying streams by scanning. 
  ///////////////////////////////////////////////////////////////////////////
	template<class T>
	class identity_scan : public scan_object {

	public:
	  err initialize(void) { 
		return NO_ERROR; 
	    }
	    
	    err operate(const T &in,
			SCAN_FLAG *sfin,
			T *out, 
			SCAN_FLAG *sfout);

	};

	template<class T>
	err identity_scan<T>::operate(const T &in, 
				      SCAN_FLAG *sfin,
                                      T *out, 
				      SCAN_FLAG *sfout)
	{
	    if ((*sfout = *sfin)) {
		*out = in;
		return SCAN_CONTINUE;
	    } 
	    else {
		return SCAN_DONE;
	    }
	};

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
	
  ///////////////////////////////////////////////////////////////////////////
  /// A copy function for streams
  ///////////////////////////////////////////////////////////////////////////
	template<class T>
	err copy_stream(stream<T> *t, stream<T> *s) {
	    
	    identity_scan<T> id;
	    
	    return scan(t, &id, s);
	}

    }  //  ami namespace

}  //  tpie namespace

#endif // _TPIE_AMI_SCAN_H 

