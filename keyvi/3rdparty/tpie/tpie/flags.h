// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 20015 The TPIE development team
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
/// \file types.h  Typesafe bitflags.
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_FLAGS_H
#define _TPIE_FLAGS_H

#include <cstdint>
#include <tpie/serialization2.h>

namespace tpie {
namespace bits {
	
template <int S>
struct enum_storage_type_help {};

template <>
struct enum_storage_type_help<1> {typedef uint8_t type;};

template <>
struct enum_storage_type_help<2> {typedef uint16_t type;};

template <>
struct enum_storage_type_help<4> {typedef uint32_t type;};

template <>
struct enum_storage_type_help<8> {typedef uint64_t type;};
  
template <typename T>
struct enum_storage_type {
	typedef typename enum_storage_type_help<sizeof(T)>::type type;
};
} //namespace bits

/**
 * \brief Type safe bitflags over an enumeration T.
 *
 * Example:
 * \code{.cpp}
 * enum OPTION {a=1, b=2, c=4};
 * TPIE_DECLARE_OPERATORS_FOR_FLAGS(OPTION)
 * typedef tpie::flags<OPTION> OPTIONS;
 * 
 * void foo(OPTIONS o) {
 *   if (o & a) do_something();
 * }
 *
 * void bar() {
 *   foo(a | c);
 * }
 */
template <typename T>
class flags {
private:
	typedef typename bits::enum_storage_type<T>::type st;
	typedef void (flags::*bool_type)() const;
	void bool_type_true() const {}
public:
	flags(): m_value(0) {}
	flags(T t): m_value(static_cast<st>(t)) {}
	friend flags operator|(const flags & l, const T & r) {return l | flags(r);}
	friend flags operator|(const flags & l, const flags & r) {return flags(l.m_value | r.m_value);}
	friend flags operator|(const T & l, const flags & r) {return flags(l) | r;}
	friend flags operator&(const flags & l, const flags & r) {return flags(l.m_value & r.m_value);}
	friend flags operator&(const T & l, const flags & r) {return flags(flags(l).m_value & r.m_value);}
	friend flags operator&(const flags & l, const T & r) {return flags(l.m_value & flags(r).m_value);}
	flags & operator|=(const T & r) { return *this |= flags(r); }
	flags & operator|=(const flags & r) { m_value |= r.m_value; return *this; }
	// See http://www.artima.com/cppsource/safebool2.html
	operator bool_type() const {return m_value?&flags::bool_type_true:NULL;}
	flags operator~() const {return flags(~m_value);}
	
	template <typename D>
	friend void serialize(D & dst, const flags<T> & f) {
		serialize(dst, f.m_value);
	}

	template <typename S>
	friend void unserialize(S & src, flags<T> & f) {
		unserialize(src, f.m_value);
	}
private:
	explicit flags(st value): m_value(value) {}
	st m_value;
};

#define TPIE_DECLARE_OPERATORS_FOR_FLAGS(T) \
inline tpie::flags<T> operator|(const T & l, const T & r) {return tpie::flags<T>(l) | r;} \
inline tpie::flags<T> operator~(const T & l) {return ~tpie::flags<T>(l);}


} //namespace tpie

#endif //#_TPIE_FLAGS_H
