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
/// \file tpie/maybe.h 
///////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_MAYBE_H__
#define __TPIE_MAYBE_H__

#include <tpie/exception.h>
#include <tpie/config.h>

namespace tpie {

template <typename T>
class maybe {
public:
	///////////////////////////////////////////////////////////////////////////////
	/// \brief Construct a new unconstructed maybe object.
	///////////////////////////////////////////////////////////////////////////////
	maybe() : m_constructed(false) {
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief If o is constructed throw an exception otherwise do nothing.
	///////////////////////////////////////////////////////////////////////////////
	maybe(const maybe<T> & o) : m_constructed(false) {
		o.assert_not_constructed();
	}

	maybe(maybe<T> && o) : m_constructed(false) {
		o.assert_not_constructed();
	}

private:
	void assert_not_constructed() const {
		if(m_constructed)
			throw maybe_exception("Maybe object already constructed");
	}
public:

	///////////////////////////////////////////////////////////////////////////////
	/// \brief If o is constructed throw an exception otherwise return *this.
	///////////////////////////////////////////////////////////////////////////////
	maybe & operator=(const maybe<T> &o) {
		o.assert_not_constructed();
		assert_not_constructed();

		return *this;
	}

	maybe & operator==(maybe<T> &o) {
		o.assert_not_constructed();
		assert_not_constructed();

		return *this;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \return Whether the object contained is constructed or not.
	///////////////////////////////////////////////////////////////////////////////
	bool is_constructed() const {
		return m_constructed;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Contains an element of the type given as template parameter and
	/// disallows copy constructing of the maybe class when the object is allocated.
	///
	///
	/// The implementation of maybe either uses variadic templates (if supported
	/// by the compiler) or a bunch of overloads to support a variable
	/// number of constructor parameters.
	///
	/// \tparam Args The variadic number of types of constructor parameters.
	/// \param args The variadic number of arguments to pass to the constructor of
	///////////////////////////////////////////////////////////////////////////////

	template <typename ... T_ARGS>
	void construct(T_ARGS && ... t) {
		assert_not_constructed();

		new(&object) T(std::forward<T_ARGS>(t)...);
		m_constructed = true;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \return A reference to the object contained.
	///////////////////////////////////////////////////////////////////////////////
	T & operator*() {
		return object;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \return A const reference to the object contained.
	///////////////////////////////////////////////////////////////////////////////
	const T & operator*() const {
		return object;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \return A pointer to the object contained.
	///////////////////////////////////////////////////////////////////////////////
	T * operator->() {
		return &object;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \return A const pointer to the object contained.
	///////////////////////////////////////////////////////////////////////////////
	const T * operator->() const {
		return &object;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Invokes the deconstructor on the object contained.
	///////////////////////////////////////////////////////////////////////////////
	void destruct() {
		if(!m_constructed)
			return;

		object.~T();
		m_constructed = false;
	}

	///////////////////////////////////////////////////////////////////////////////
	/// \brief Calls the destruct method and destroys the instance.
	///////////////////////////////////////////////////////////////////////////////
	~maybe() {
		destruct();
	}
private:
	union { T object; };
	bool m_constructed;
};
} // namespace tpie

#endif
