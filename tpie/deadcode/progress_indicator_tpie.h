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

#ifndef _TPIE_PROGRESS_INDICATOR_TPIE_H
#define _TPIE_PROGRESS_INDICATOR_TPIE_H

#include <portability.h>
#include <algorithm>

#include <progress_indicator_terminal.h>

namespace tpie {

///////////////////////////////////////////////////////////////////
///
///  A class that indicates the progress by expanding the
///  word tpie.
///
///////////////////////////////////////////////////////////////////
    
    class progress_indicator_tpie : public progress_indicator_terminal {
	
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
	
	progress_indicator_tpie(const std::string& title, 
							const std::string& description, 
							TPIE_OS_OFFSET minRange, 
							TPIE_OS_OFFSET maxRange, 
							TPIE_OS_OFFSET stepValue) : 
	    progress_indicator_terminal(title, description, minRange, maxRange, stepValue), m_indicatorLength(0), m_progress(0) {
	    m_indicatorLength = 40;
	}
	
  ////////////////////////////////////////////////////////////////////
  ///  Copy-constructor.
  ////////////////////////////////////////////////////////////////////

	progress_indicator_tpie(const progress_indicator_tpie& other) : 
	    progress_indicator_terminal(other), m_indicatorLength(40), m_progress(0) {
	    *this = other;
	}
	
  ////////////////////////////////////////////////////////////////////
  ///  Assignment operator.
  ////////////////////////////////////////////////////////////////////

	progress_indicator_tpie& operator=(const progress_indicator_tpie& other) {
	    if (this != &other) {
		
		progress_indicator_terminal::operator=(other);
		
		m_indicatorLength = other.m_indicatorLength;
		m_progress        = other.m_progress;
	    }
	    return *this;
	}
	
	////////////////////////////////////////////////////////////////////
	///
	///  The destructor. Nothing is done.
	///
	////////////////////////////////////////////////////////////////////
	
	virtual ~progress_indicator_tpie() {
	    // Do nothing.
	};
	
	////////////////////////////////////////////////////////////////////
	///
	///  Set the maximum length of the indicator. The length is enforced
	///  to be an integer in [2,60].
	///
	///  \param  indicatorLength  The maximum length of the indicator.
	///
	////////////////////////////////////////////////////////////////////
	
	void set_indicator_length(int indicatorLength) {
	    m_indicatorLength = max(2, min(60, indicatorLength));
	}
	
	////////////////////////////////////////////////////////////////////
	///
	///  Reset the current state of the indicator and its current length
	///
	////////////////////////////////////////////////////////////////////
	
	virtual void reset() {
	    m_current  = m_minRange;
	    m_progress = 0;
	}
	
	////////////////////////////////////////////////////////////////////
	///
	///  Display the indicator.
	///
	////////////////////////////////////////////////////////////////////
	
	void refresh() {
	    //  Compute the relative length of the arrow.
	    TPIE_OS_OFFSET progress = m_indicatorLength * 
		(m_current-m_minRange)/(m_maxRange-m_minRange); 
	    
	    //  Make sure that the first item gets printed.
	    if (progress == 0) progress = 1;
	    
	    //  Only print stuff to std::cout if the indicator needs to be updated.
	    if (progress > m_progress) {
		
		//  Don't print the last item.
		if (progress == m_indicatorLength) progress--;
		
		//  Go to the beginning of the line and print the description.
		cout << "\r" << m_description << " [t";
		
		//  Extend the e's.
		for(TPIE_OS_OFFSET i = 0; i < progress; i++) std::cout << "e";
		cout << "pie";
		
		//  Print blank space.
		for(TPIE_OS_OFFSET i = progress+1; i < m_indicatorLength; i++) std::cout << " ";
		cout << "] ";
		
		//  Print either a percentage sign or the maximum range.
		display_percentage();
		
		cout << flush;
		m_progress = progress;
	    }
	}
	
    protected:
	
	
	/** The maximal length of the indicator */
	TPIE_OS_OFFSET m_indicatorLength;
	
	/** The current length of the indicator */
	TPIE_OS_OFFSET m_progress;
	
	
    private:
	progress_indicator_tpie();
    };
    
}  //  tpie namespace

#endif // _TPIE_PROGRESS_INDICATOR_TPIE
