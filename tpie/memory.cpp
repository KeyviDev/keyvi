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
#include <cstdlib>
#include "pretty_print.h"

namespace tpie {

inline void segfault() {
	std::abort();
}

memory_manager * mm = 0;

memory_manager::memory_manager(): m_limit(0), m_maxExceeded(0), m_nextWarning(0), m_enforce(ENFORCE_WARN) {}

size_t memory_manager::used() const throw() {
	return m_used.fetch();
}

size_t memory_manager::available() const throw() {
	size_t used = m_used.fetch();
	size_t limit = m_limit;
	if (used < limit) return limit-used;
	return 0;
}

} // namespace tpie

void tpie_print_memory_complaint(std::ostream & os, size_t bytes, size_t usage, size_t limit) {
	os << "Memory limit exceeded by " << tpie::bits::pretty_print::size_type(usage - limit)
	   << " (" << (usage-limit) * 100 / limit << "%), while trying to allocate " << tpie::bits::pretty_print::size_type(bytes) << "."
	   << " Limit is " << tpie::bits::pretty_print::size_type(limit) << ", but " << tpie::bits::pretty_print::size_type(usage) << " would be used.";
}

namespace tpie {

void memory_manager::register_allocation(size_t bytes) {
	switch(m_enforce) {
	case ENFORCE_IGNORE:
		m_used.add(bytes);
		break;
	case ENFORCE_THROW: {
		size_t usage = m_used.add_and_fetch(bytes);
		if (usage > m_limit && m_limit > 0) {
			std::stringstream ss;
			tpie_print_memory_complaint(ss, bytes, usage, m_limit);
			throw out_of_memory_error(ss.str().c_str());
		}
		break; }
	case ENFORCE_DEBUG:
	case ENFORCE_WARN: {
		size_t usage = m_used.add_and_fetch(bytes);
		if (usage > m_limit && usage - m_limit > m_maxExceeded && m_limit > 0) {
			m_maxExceeded = usage - m_limit;
			if (m_maxExceeded >= m_nextWarning) {
				m_nextWarning = m_maxExceeded + m_maxExceeded/8;
				std::ostream & os = (m_enforce == ENFORCE_DEBUG) ? log_debug() : log_warning();
				tpie_print_memory_complaint(os, bytes, usage, m_limit);
				os << std::endl;
			}
		}
		break; }
	};
}

void memory_manager::register_deallocation(size_t bytes) {
#ifndef TPIE_NDEBUG
	size_t usage = m_used.fetch_and_sub(bytes);
	if (bytes > usage) {
		log_error() << "Error in deallocation, trying to deallocate " << bytes << " bytes, while only " <<
			usage << " were allocated" << std::endl;
		segfault();
	}
#else
	m_used.sub(bytes);
#endif
}


void memory_manager::set_limit(size_t new_limit) {
	m_limit = new_limit;
}

void memory_manager::set_enforcement(enforce_t e) {
	m_enforce = e;
}

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Buffers messages to the debug log.
/// TPIE logging might use the memory manager. We don't allow memory
/// allocation/deallocation while doing an allocation/deallocation, so messages
/// stored in the log_flusher aren't sent to the TPIE log until we are done
/// allocating/deallocating.
///////////////////////////////////////////////////////////////////////////////
struct log_flusher {
	std::stringstream buf;
	~log_flusher() {
		std::string msg = buf.str();
		if(!msg.empty()) {
			tpie::log_debug() << msg;
			tpie::log_debug().flush();
		}
	}
};

std::pair<uint8_t *, size_t> memory_manager::__allocate_consecutive(size_t upper_bound, size_t granularity) {
	log_flusher lf;

	size_t high=available()/granularity;
	if (upper_bound != 0) 
		high=std::min(upper_bound/granularity, high);
	size_t low=1;
	uint8_t * res;

	//first check quickly if we can get "high" bytes of memory
	//directly.
	try {
		res = new uint8_t[high*granularity];
		m_used.add(high*granularity);
#ifndef TPIE_NDEBUG
		register_pointer(res, high*granularity, typeid(uint8_t) );
#endif	      
		return std::make_pair(res, high*granularity);
	} catch (std::bad_alloc) {
		lf.buf << "Failed to get " << (high*granularity)/(1024*1024) << " megabytes of memory. "
		 	   << "Performing binary search to find largest amount "
			   << "of memory available. This might take a few moments.\n";
	}

	//perform a binary search in [low,high] for highest possible 
	//memory allocation
	size_t best=0;
	do {
		size_t mid = static_cast<size_t>((static_cast<uint64_t>(low)+high)/2);

		lf.buf << "Search area is  [" << low*granularity << "," << high*granularity << "]"
			   << " query amount is: " << mid*granularity << ":\n";

		//try to allocate "mid" bytes of memory
		//std::new throws an exception if memory allocation fails
		try {
			uint8_t* mem = new uint8_t[mid*granularity];
			low = mid+1;
			best=mid*granularity;
			delete[] mem;
		} catch (std::bad_alloc) {
			high = mid-1;
			lf.buf << "   failed.\n";
		}
	} while (high >= low);
	lf.buf << "- - - - - - - END MEMORY SEARCH - - - - - -\n";	

	res = new uint8_t[best];
	m_used.add(best);
#ifndef TPIE_NDEBUG
	register_pointer(res, best, typeid(uint8_t) );
#endif	      
	return std::make_pair(res, best);
}


#ifndef TPIE_NDEBUG
void memory_manager::register_pointer(void * p, size_t size, const std::type_info & t) {
	boost::mutex::scoped_lock lock(m_mutex);
	__register_pointer(p, size, t);
}

void memory_manager::__register_pointer(void * p, size_t size, const std::type_info & t) {
	if (m_pointers.count(p) != 0) {
		log_error() << "Trying to register pointer " << p << " of size " 
					<< size << " which is already registered" << std::endl;
		segfault();
	}
	m_pointers[p] = std::make_pair(size, &t);;
}

void memory_manager::unregister_pointer(void * p, size_t size, const std::type_info & t) {
	boost::mutex::scoped_lock lock(m_mutex);
	__unregister_pointer(p, size, t);
}

void memory_manager::__unregister_pointer(void * p, size_t size, const std::type_info & t) {
	boost::unordered_map<void *, std::pair<size_t, const std::type_info *> >::const_iterator i=m_pointers.find(p);
	if (i == m_pointers.end()) {
		log_error() << "Trying to deregister pointer " << p << " of size "
					<< size << " which was never registered" << std::endl;
		segfault();
	} else {
		if (i->second.first != size) {
			log_error() << "Trying to deregister pointer " << p << " of size "
						<< size << " which was registered with size " << i->second.first << std::endl;
			segfault();
		}
		if (*i->second.second != t) {
			log_error() << "Trying to deregister pointer " << p << " of type "
						<< t.name() << " which was registered with size " << i->second.second->name() << std::endl;
			segfault();
		}
		m_pointers.erase(i);
	}
}

void memory_manager::assert_tpie_ptr(void * p) {
	boost::mutex::scoped_lock lock(m_mutex);
	__assert_tpie_ptr(p);
}

void memory_manager::__assert_tpie_ptr(void * p) {
	if (!p || m_pointers.count(p)) return;
	log_error() << p << " has not been allocated with tpie_new" << std::endl;
	segfault();
}

void memory_manager::complain_about_unfreed_memory() {
	boost::mutex::scoped_lock lock(m_mutex);
	__complain_about_unfreed_memory();
}

void memory_manager::__complain_about_unfreed_memory() {
	if(m_pointers.size() == 0) return;
	log_error() << "The following pointers were either leaked or deleted with delete instead of tpie_delete" << std::endl << std::endl;
	
	for(boost::unordered_map<void *, std::pair<size_t, const std::type_info *> >::const_iterator i=m_pointers.begin();
		i != m_pointers.end(); ++i)
		log_error() << "  " <<  i->first << ": " << i->second.second->name() << " of " << i->second.first << " bytes" << std::endl;
}
#endif

void init_memory_manager() {
	mm = new memory_manager();
}

void finish_memory_manager() {
#ifndef TPIE_NDEBUG
	mm->complain_about_unfreed_memory();
#endif
	delete mm;
	mm = 0;
}

memory_manager & get_memory_manager() {
#ifndef TPIE_NDEBUG
	if (mm == 0) throw std::runtime_error("Memory management not initialized");
#endif
	return * mm;
}

size_t consecutive_memory_available(size_t granularity) {
	std::pair<uint8_t *, size_t> r = get_memory_manager().__allocate_consecutive(0, granularity);
	tpie_delete_array(r.first, r.second);
	return r.second;
}


} //namespace tpieg
