// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2012, The TPIE development team
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
/// \file concept_doc.h
/// \brief Concept documentation.
///////////////////////////////////////////////////////////////////////////////
namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \brief Description of the methods that must be implemented to support the
/// linear_memory_structure_concept.
///////////////////////////////////////////////////////////////////////////////
struct linear_memory_structure_doc {
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the memory coefficient of the structure
	/// 
	/// Allocating a structure with n elements will use at most
	/// \f$ \lfloor \mathrm{memory\_coefficient} \cdot n + \mathrm{memory\_overhead} \rfloor \f$
	/// bytes. This does not include memory overhead incurred if the structure
	/// is allocated using new.
	/// \return The memory coefficient of the structure.
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the memory overhead of the structure
	/// 
	/// \sa memory_coefficient()
	/// \return The memory overhead.
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead();
};

}

