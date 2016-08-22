// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2011, 2014, The TPIE development team
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

#include "memory.h"
#include <iostream>
#include <sstream>
#include "tpie_log.h"
#include <cstring>
#include "pretty_print.h"

namespace tpie {

resource_manager::resource_manager(resource_type type)
	: m_used(0), m_limit(0), m_maxExceeded(0), m_nextWarning(0), m_enforce(ENFORCE_WARN), resource_managed(type) {}

size_t resource_manager::used() const noexcept {
	return m_used.load();
}

size_t resource_manager::available() const noexcept {
	size_t used = m_used.load();
	size_t limit = m_limit;
	if (used < limit) return limit-used;
	return 0;
}

void resource_manager::print_resource_complaint(std::ostream & os, size_t amount, size_t usage) {
	size_t diff = usage - m_limit;

	os << "Resource " << resource_managed << " limit exceeded by " << amount_with_unit(diff)
	   << " (" << (diff * 100 / m_limit) << "%), while trying to increase usage by " << amount_with_unit(amount) << "."
	   << " Limit is " << amount_with_unit(m_limit) << ", but " << amount_with_unit(usage) << " would be used.";
}

void resource_manager::register_increased_usage(size_t amount) {
	switch(m_enforce) {
	case ENFORCE_IGNORE:
		m_used.fetch_add(amount);
		break;
	case ENFORCE_THROW: {
		size_t usage = m_used.fetch_add(amount) + amount;
		if (usage > m_limit && m_limit > 0) {
			std::stringstream ss;
			print_resource_complaint(ss, amount, usage);
			throw_out_of_resource_error(ss.str());
			throw out_of_resource_error(ss.str());
		}
		break; }
	case ENFORCE_DEBUG:
	case ENFORCE_WARN: {
		size_t usage = m_used.fetch_add(amount) + amount;
		if (usage > m_limit && usage - m_limit > m_maxExceeded && m_limit > 0) {
			m_maxExceeded = usage - m_limit;
			if (m_maxExceeded >= m_nextWarning) {
				m_nextWarning = m_maxExceeded + m_maxExceeded/8;
				std::ostream & os = (m_enforce == ENFORCE_DEBUG) ? log_debug() : log_warning();
				print_resource_complaint(os, amount, usage);
				os << std::endl;
			}
		}
		break; }
	};
}

void resource_manager::register_decreased_usage(size_t amount) {
#ifndef TPIE_NDEBUG
	size_t usage = m_used.fetch_sub(amount);
	if (amount > usage) {
		log_error() << "Error in decrease_usage, trying to decrease by "
		            << amount_with_unit(amount) << " , while only "
		            << amount_with_unit(usage) << " were allocated" << std::endl;
		std::abort();
	}
#else
	m_used.fetch_sub(amount);
#endif
}

void resource_manager::set_limit(size_t new_limit) {
	m_limit = new_limit;
}

void resource_manager::set_enforcement(enforce_t e) {
	m_enforce = e;
}

} //namespace tpie
