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
/// \file progress_indicator_spin.h  Indicate progress by a spinning cross
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_PROGRESS_INDICATOR_SPIN_H
#define _TPIE_PROGRESS_INDICATOR_SPIN_H

#include <tpie/portability.h>
#include <string.h>
#include <algorithm>

#include <tpie/progress_indicator_terminal.h>

namespace tpie {

///////////////////////////////////////////////////////////////////
///
///  A class that indicates the progress by a spinning cross.
///
///////////////////////////////////////////////////////////////////

    class progress_indicator_spin : public progress_indicator_terminal {

    public:

	////////////////////////////////////////////////////////////////////
	///
	///  Initializes the indicator.
	///
	///  \param  title        The title of the progress indicator.
	///  \param  description  A text to be printed in front of the 
	///                       indicator.
	///  \param  minRange     The lower bound of the range.
	///  \param  maxRange     The upper bound of the range.
	///  \param  stepValue    The increment for each step.
	///
	////////////////////////////////////////////////////////////////////

	progress_indicator_spin(const std::string& title, 
							const std::string& description, 
							stream_size_type minRange, 
							stream_size_type maxRange, 
							stream_size_type stepValue) : 
	    progress_indicator_terminal(title, description, minRange, maxRange, stepValue), m_symbols(NULL), m_numberOfStates(0), m_state(0) {
	    m_numberOfStates = 4;
	    m_symbols = new char[m_numberOfStates+2];
	    m_symbols[0] = '|';
	    m_symbols[1] = '/';
	    m_symbols[2] = '-';
	    m_symbols[3] = '\\';
	    m_symbols[4] = 'X';
	    m_symbols[5] = '\0';
	}

  ////////////////////////////////////////////////////////////////////
  ///  Copy-constructor.
  ////////////////////////////////////////////////////////////////////
	
	progress_indicator_spin(const progress_indicator_spin& other) : 
	    progress_indicator_terminal(other), m_symbols(NULL), m_numberOfStates(0), m_state(0) {
	    *this = other;
	}

  ////////////////////////////////////////////////////////////////////
  ///  Assignment operator.
  ////////////////////////////////////////////////////////////////////
	
	progress_indicator_spin& operator=(const progress_indicator_spin& other) {
	    if (this != &other) {

		progress_indicator_terminal::operator=(other);

		m_numberOfStates = other.m_numberOfStates;
		m_state          = other.m_state;
	    
		delete[] m_symbols;

		m_symbols = new char[m_numberOfStates+2];
		memcpy(m_symbols, other.m_symbols, m_numberOfStates+2);
	    }
	    return *this;
	}
    
	////////////////////////////////////////////////////////////////////
	///
	///  The destructor. Nothing is done.
	///
	////////////////////////////////////////////////////////////////////

	virtual ~progress_indicator_spin() {
	    delete [] m_symbols;
	};
    
	////////////////////////////////////////////////////////////////////
	///
	///  Display the indicator.
	///
	////////////////////////////////////////////////////////////////////

	virtual void refresh() {
		m_state = ++m_state % m_numberOfStates;

		//  Use the last symbol for indicating "done".
		if (m_current == m_maxRange) m_state = m_numberOfStates;

		//  Go to the beginning of the line and print the description.
		std::cout << "\r" << m_description << " " << m_symbols[m_state] << std::flush;
	}

    protected:

	/**  The characters used for the spinning indicator.  */
	char* m_symbols;

	/**  The number of characters used for the spinning indicator.  */
	unsigned short m_numberOfStates;

	/**  The current character used for the spinning indicator.  */
	unsigned short m_state;

    private:
	progress_indicator_spin();
    };

}  //  tpie namespace

#endif // _TPIE_PROGRESS_INDICATOR_SPIN_H
