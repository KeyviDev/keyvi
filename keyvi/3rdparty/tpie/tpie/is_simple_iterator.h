// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#include <vector>
#include <string>

namespace tpie {

template <bool> struct is_simple_iterator_enable_if { };
template <> struct is_simple_iterator_enable_if<true> {typedef void type;};

/**
 * \brief Checks if an iterator is simple
 *
 * An iterator is simple if it is a random access iterator
 * and all elements in a range from a to b
 * are located from &(*a) to &(*b)
 */
template <typename T>
class is_simple_iterator {
private:
  template <typename TT>
  static char magic(typename std::vector<typename TT::value_type>::iterator*);
  template <typename TT>
  static char magic(typename std::vector<typename TT::value_type>::const_iterator*);
  template <typename TT>
  static char magic(typename std::basic_string<char>::iterator*);
  template <typename TT>
  static char magic(typename std::basic_string<char>::const_iterator*);
  template <typename TT>
  static char magic(TT *, typename is_simple_iterator_enable_if<TT::is_simple_iterator>::type *_=0);
  template <typename TT>
  static long magic(...);
public:
  static bool const value=sizeof(magic<T>((T*)0))==sizeof(char);
};

} //namespace tpie
