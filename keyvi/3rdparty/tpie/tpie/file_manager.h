// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2011, The TPIE development team
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
/// \file tpie/file.h Memory management subsystem.
///////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_FILE_MANAGER_H__
#define __TPIE_FILE_MANAGER_H__

#include <tpie/config.h>
#include <tpie/util.h>
#include <tpie/resource_manager.h>
#include <mutex>
#include <unordered_map>
#include <type_traits>
#include <utility>
#include <atomic>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief File management object used to track file usage.
///////////////////////////////////////////////////////////////////////////////
class file_manager final : public resource_manager {
public:
	///////////////////////////////////////////////////////////////////////////
	/// \internal
	/// Construct the file manager object.
	///////////////////////////////////////////////////////////////////////////
	file_manager();

	void increment_open_file_count() {
		register_increased_usage(1);
	}

	void decrement_open_file_count() {
		register_decreased_usage(1);
	}

	std::string amount_with_unit(size_t amount) const override {
		std::ostringstream os;
		if (amount == 1) {
			os << "a file";
		} else {
			os << amount << " files";
		}
		return os.str();
	}

protected:
	void throw_out_of_resource_error(const std::string & s) override {
		throw out_of_files_error(s);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_init to initialize the file manager.
///////////////////////////////////////////////////////////////////////////////
void init_file_manager();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_finish to deinitialize the file manager.
///////////////////////////////////////////////////////////////////////////////
void finish_file_manager();

///////////////////////////////////////////////////////////////////////////////
/// \brief Return a reference to the file manager.
/// May only be called when init_file_manager has been called.
/// See \ref tpie_init().
///////////////////////////////////////////////////////////////////////////////
file_manager & get_file_manager();

} //namespace tpie

#endif //__TPIE_MEMORY_H__
