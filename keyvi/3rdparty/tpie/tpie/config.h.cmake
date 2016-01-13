#ifndef _CONFIG_H
#define _CONFIG_H

#include <boost/version.hpp>

#cmakedefine TPIE_HAVE_UNISTD_H
#cmakedefine TPIE_HAVE_SYS_UNISTD_H

#cmakedefine TPIE_DEPRECATED_WARNINGS
#cmakedefine TPIE_PARALLEL_SORT

#if defined (TPIE_HAVE_UNISTD_H)
#include <unistd.h>
#elif defined(TPIE_HAVE_SYS_UNISTD_H)
#include <sys/unistd.h>
#endif

// On Solaris, _SC_PAGE_SIZE is called _SC_PAGE_SIZE.  Here's a quick
// fix.
#if !defined(_SC_PAGE_SIZE) && defined(_SC_PAGESIZE)
#define _SC_PAGE_SIZE _SC_PAGESIZE
#endif

#cmakedefine TPL_LOGGING 1

#cmakedefine TPIE_NDEBUG

#cmakedefine TPIE_FRACTIONDB_DIR_INL
#ifdef TPIE_FRACTIONDB_DIR_INL
#undef TPIE_FRACTIONDB_DIR_INL
#define TPIE_FRACTIONDB_DIR_INL "${TPIE_FRACTIONDB_DIR_INL}"
#endif

#ifdef WIN32
	//disable windows crt security and deprecation warnings
	#define _CRT_SECURE_NO_DEPRECATE 
	#define _CRT_NONSTDC_NO_DEPRECATE
	#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER
	// recent visual studio versions gives heaps of compilation
	// warnings about security issues with fopen/strcpy and the like
	// this disables these warnings.
	#pragma warning(disable : 4996)

	// We know that visual studio does not know what to do with throw() 
	// but realy dont care
	#pragma warning(disable : 4290)

	// We know that we are casting ints to bool
	#pragma warning(disable : 4800)
#endif

#ifdef _MSC_VER
#define tpie_unreachable() __assume(0)
#else
#define tpie_unreachable() __builtin_unreachable()
#endif

//We use boost filesystem v3 from boost v1.47 and onwards
//(boost v 1.50 dropped fs v2 altogether)
#if BOOST_VERSION >= 104700
	#define BOOST_FILESYSTEM_VERSION 3
#else
	#define BOOST_FILESYSTEM_VERSION 2
#endif

#cmakedefine TPIE_HAS_SNAPPY

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  //ensures that the windows min/max macros are not defined 
#endif
#endif

#endif // _CONFIG_H 
