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

///////////////////////////////////////////////////////////////////////////////
/// \file progress_indicator_arrow.h  Indicate progress by expanding an arrow
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_PROGRESS_INDICATOR_ARROW_H
#define _TPIE_PROGRESS_INDICATOR_ARROW_H

#include <algorithm>
#include <iostream>
#include <tpie/progress_indicator_terminal.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \class progress_indicator_arrow
/// A class that indicates the progress by expanding an arrow.
///////////////////////////////////////////////////////////////////////////////
    class progress_indicator_arrow : public progress_indicator_terminal {
	private:
		progress_indicator_arrow(const progress_indicator_arrow& other);
    public:

	///////////////////////////////////////////////////////////////////////////
	/// Initializes the indicator.
	///
	/// \param  title  The title of the progress indicator.
	/// \param  range  The number of times we call step
	///////////////////////////////////////////////////////////////////////////
	progress_indicator_arrow(const char * title, stream_size_type range, std::ostream & os = std::cout) :
	    progress_indicator_terminal(title, range, os) , m_indicatorLength(0), m_os(os) {
	    m_indicatorLength = 110;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Set the maximum length of the indicator. The length is enforced
	/// to be an integer in [2,60].
	///
	/// \param  indicatorLength  The maximum length of the indicator.
	///////////////////////////////////////////////////////////////////////////
	void set_indicator_length(int indicatorLength) {
	    m_indicatorLength = std::max(2, std::min(60, indicatorLength));
	}

	///////////////////////////////////////////////////////////////////////////
	/// Reset the current state of the indicator and its current length
	///////////////////////////////////////////////////////////////////////////
	virtual void reset() {
	    m_current  = 0;
	}

	void push_breadcrumb(const char * crumb, description_importance /*importance*/) {
		m_crumbs.push_back(crumb);
	}

	void pop_breadcrumb() {
		m_crumbs.pop_back();
	}

	///////////////////////////////////////////////////////////////////////////
	/// Display the indicator.
	///////////////////////////////////////////////////////////////////////////
	virtual void refresh() {
	    //  Compute the relative length of the arrow.
		//std::cout << "refresh " << m_description << std::endl;

		memory_size_type l = m_indicatorLength - 12  - m_title.size();
		memory_size_type progress = (m_range != 0) ?
			static_cast<memory_size_type>(l * m_current / m_range) : 0;

		std::string newStatus;
		{
			std::stringstream status;

			//  Don't print the last item.
			if (progress >= l) progress = l -1;

			//  Go to the beginning of the line and print the description.
			status << m_title << " [";

			//  Extend the arrow.

			status << std::string(progress, '=');
			status << '>';

			//  Print blank space.
			status << std::string(l-progress-1, ' ');
			status << "] ";

			status << m_current * 100 / m_range << '%';

			for (std::deque<std::string>::iterator i = m_crumbs.begin(); i != m_crumbs.end(); ++i) {
				if (i == m_crumbs.begin()) status << ' ';
				else status << " > ";
				status << *i;
			}

			status << ' ' << estimated_remaining_time();

			newStatus = status.str();
		}

		if (newStatus != m_status) {
#ifdef WIN32
			m_os << '\r' << std::string(m_status.size(), ' ') << '\r'
				<< newStatus << std::flush;
#else
			m_os << "\r\x1B[K" << newStatus << std::flush;
#endif
			std::swap(newStatus, m_status);
		}
	}

    protected:

	/** The maximal length of the indicator */
	memory_size_type m_indicatorLength;

	/** The previously displayed status line. */
	std::string m_status;

	/** ostream on which to display the progress indicator */
	std::ostream & m_os;

	std::deque<std::string> m_crumbs;

    private:

	progress_indicator_arrow();
    };

}

#endif // _TPIE_PROGRESS_INDICATOR_ARROW
