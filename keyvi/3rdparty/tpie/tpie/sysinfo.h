// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file sysinfo.h System and platform runtime information
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_SYSINFO__
#define __TPIE_SYSINFO__

#include <iostream>
#include <iomanip>
#include <boost/asio/ip/host_name.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <tpie/file.h> // for block size
#include <tpie/tpie_log.h>

namespace tpie {

/** \brief The Git commit hash (40 hexadecimal characters) that TPIE was built
 * from. */
extern const char * git_commit;

/** The Git refspec that TPIE was built from. Usually of the form
 * \c "refs/heads/"+branch, for instance \c "refs/heads/master". */
extern const char * git_refspec;

///////////////////////////////////////////////////////////////////////////////
/// \brief Class providing system and platform info.
///////////////////////////////////////////////////////////////////////////////
struct sysinfo {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor.
	///////////////////////////////////////////////////////////////////////////
	inline sysinfo()
		: m_platform(calc_platform())
		, m_hostname(calc_hostname())
		, m_blocksize(calc_blocksize())
	{
		// Does nothing.
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Git commit hash.
	/// \sa tpie::git_commit
	///////////////////////////////////////////////////////////////////////////
	inline std::string commit()    const { return git_commit; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Git refspec.
	/// \sa tpie::git_refspec
	///////////////////////////////////////////////////////////////////////////
	inline std::string refspec()   const { return git_refspec; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Platform description. Currently \c "Windows" or \c "Linux"
	/// followed by \c "32-bit" or \c "64-bit" depending on the \c WIN32
	/// compile-time define and \c sizeof(size_t).
	///////////////////////////////////////////////////////////////////////////
	inline std::string platform()  const { return m_platform; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief System hostname as reported by Boost ASIO.
	///////////////////////////////////////////////////////////////////////////
	inline std::string hostname()  const { return m_hostname; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Block size used by \ref tpie::ami::stream.
	///////////////////////////////////////////////////////////////////////////
	inline std::string blocksize() const { return m_blocksize; }

	///////////////////////////////////////////////////////////////////////////
	/// \brief Local date and time in a human-readable format.
	///////////////////////////////////////////////////////////////////////////
	inline std::string localtime() const {
		boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
		return to_simple_string(now);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Helper function to make a custom key-value line.
	/// \code
	/// sysinfo i;
	/// std::cout << i;
	/// std::cout << i.custominfo("Verbosity", m_verbose ? "On" : "Off")
	///           << std::endl
	///           << i.custominfo("", "Starting test") << std::endl;
	/// \endcode
	/// could print out
	/// \code
	/// "Hostname:       sanford"
	/// "Platform:       Linux 64-bit"
	/// "Git branch:     refs/heads/master"
	/// "Git commit:     5f0bebba1f4b2b53f4bb4d5709d3d347bc469dc9"
	/// "Local time:     2012-Mar-13 11:17:33"
	/// "Block size:     2048 KiB"
	/// "Verbosity:      On"
	/// "                Starting test"
	/// \endcode
	/// \param key Text to display in left column. Should be at most 14
	/// characters.
	/// \param value Text to display in right column. Should be at most 63
	/// characters.
	///////////////////////////////////////////////////////////////////////////
	template <typename V>
	inline std::string custominfo(std::string key, const V & value) {
		std::stringstream builder;
		if (key != "") key += ':';
		builder.flags(std::ios::left);
		builder << std::setw(16) << key << value;
		return builder.str();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Print custom info to std::cout.
	///////////////////////////////////////////////////////////////////////////
	template <typename V>
	inline void printinfo(std::string key, const V & value) {
		std::cout << custominfo(key, value) << std::endl;
	}

	static inline memory_size_type blocksize_bytes() {
		return get_block_size();
	}

private:
	static const char * m_commit;
	static const char * m_refspec;
	const std::string m_platform;
	const std::string m_hostname;
	const std::string m_blocksize;

	static inline std::string calc_platform() {
		std::stringstream p;
#ifdef WIN32
		p << "Windows ";
#else
		p << "Linux ";
#endif
		p << (8*sizeof(size_t)) << "-bit";
		return p.str();
	}

	static inline std::string calc_hostname() {
		try {
			return boost::asio::ip::host_name();
		} catch (boost::system::system_error & e) {
			log_debug() << "boost::system::system_error thrown while getting hostname. e.what() == " << e.what() << std::endl;
			return "Exception";
		}
	}

	static inline std::string calc_blocksize() {
		std::stringstream ss;
		ss << blocksize_bytes() / 1024
		   << " KiB";
		return ss.str();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Report default system info to the specified \c ostream.
/// \sa sysinfo::printinfo
///////////////////////////////////////////////////////////////////////////////
inline std::ostream & operator<<(std::ostream & s, const sysinfo & info) {
	return s
		<< "Hostname:       " << info.hostname() << '\n'
		<< "Platform:       " << info.platform() << '\n'
		<< "Git branch:     " << info.refspec() << '\n'
		<< "Git commit:     " << info.commit() << '\n'
		<< "Local time:     " << info.localtime() << '\n'
		<< "Block size:     " << info.blocksize() << '\n'
		<< "Parallel sort:  "
#ifdef TPIE_PARALLEL_SORT
		<< "Enabled"
#else
		<< "Disabled"
#endif
		<< '\n'
		<< "Snappy:         "
#ifdef TPIE_HAS_SNAPPY
		<< "Enabled"
#else
		<< "Disabled"
#endif
		<< '\n'
		;
}

}

#endif // __TPIE_SYSINFO__
