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

// Templates classes for one-, two-, and three-dimensional arrays. 
#ifndef _VARARRAY_H
#define _VARARRAY_H

#include <tpie/portability.h>

#include <cassert>
#include <cstdlib>
#include <string.h>

//----------------------------------------------------------------------

template <class T> class VarArray1D {

public:
    //  There is no default constructor.

    VarArray1D(TPIE_OS_SIZE_T dim0);
    VarArray1D(const VarArray1D& other);
    ~VarArray1D();

    VarArray1D<T>& operator=(const VarArray1D<T>& other);

    const T& operator()(TPIE_OS_SIZE_T index0) const;
    T& operator()(TPIE_OS_SIZE_T index0);

    TPIE_OS_SIZE_T size() const;
      
protected:
    T*             data;
    TPIE_OS_SIZE_T dim;
    
private:
    VarArray1D<T>() {}
    
};

//----------------------------------------------------------------------

template <class T> class VarArray2D {

public:
    //  There is no default constructor.
    VarArray2D(TPIE_OS_SIZE_T dim0, TPIE_OS_SIZE_T dim1);
    VarArray2D(const VarArray2D& other);
    ~VarArray2D();

    VarArray2D& operator=(const VarArray2D& other);

    const T& operator()(TPIE_OS_SIZE_T index0, TPIE_OS_SIZE_T index1) const;
    T& operator()(TPIE_OS_SIZE_T index0, TPIE_OS_SIZE_T index1);

    TPIE_OS_SIZE_T size() const;
    TPIE_OS_SIZE_T size(TPIE_OS_SIZE_T d) const;
      
protected:
    T*           data;
    TPIE_OS_SIZE_T dim[2];
    
private:
    VarArray2D() {}
    
};

//----------------------------------------------------------------------

template <class T> class VarArray3D {

public:
    //  There is no default constructor.
    VarArray3D(TPIE_OS_SIZE_T dim0, TPIE_OS_SIZE_T dim1, TPIE_OS_SIZE_T dim2);
    VarArray3D(const VarArray3D& other);
    ~VarArray3D();

    VarArray3D& operator=(const VarArray3D& other);

    const T& operator()(TPIE_OS_SIZE_T index0, TPIE_OS_SIZE_T index1, TPIE_OS_SIZE_T index2) const;
    T& operator()(TPIE_OS_SIZE_T index0, TPIE_OS_SIZE_T index1, TPIE_OS_SIZE_T index2);

    TPIE_OS_SIZE_T size() const;
    TPIE_OS_SIZE_T size(TPIE_OS_SIZE_T d) const;
      
protected:
    T*           data;
    TPIE_OS_SIZE_T dim[3];
    
private:
    VarArray3D() {}
    
};

//----------------------------------------------------------------------
//----------------------------------------------------------------------

template <class T>
VarArray1D<T>::VarArray1D(TPIE_OS_SIZE_T dim) {
    this->dim = dim;
    
    //  Allocate memory for dim0 elements of type/class T.
    data = tpie::tpie_new_array<T>(dim);
}

template <class T>
VarArray1D<T>::VarArray1D(const VarArray1D& other) {
    *this = other;
}

template <class T>
VarArray1D<T>::~VarArray1D() {
    // Free allocated memory.
	tpie::tpie_delete_array(data, dim);
}

template <class T>
VarArray1D<T>& VarArray1D<T>::operator=(const VarArray1D<T>& other) {
    if (this == &other) return (*this);
	tpie::tpie_delete_array(data, dim);
	this->dim = other.dim;
	
	//  Allocate memory for dim elements of type/class T.
	data = tpie::tpie_new_array<T>(dim);
	
	//  Copy objects.
	std::copy(other.data, other.data+dim, data);
    return (*this);	
}

template <class T>
const T& VarArray1D<T>::operator()(TPIE_OS_SIZE_T index0) const {
    assert(index0 < size());
    
    return data[index0];
}

template <class T>
T& VarArray1D<T>::operator()(TPIE_OS_SIZE_T index0) {
    assert(index0 < size());
    
    return data[index0];
}

template <class T>
TPIE_OS_SIZE_T VarArray1D<T>::size() const {
    return dim;
}

//----------------------------------------------------------------------

template <class T>  
VarArray2D<T>::VarArray2D(TPIE_OS_SIZE_T dim0, TPIE_OS_SIZE_T dim1) {
    this->dim[0] = dim0;
    this->dim[1] = dim1;
    
    //  Allocate memory for dim0 * dim1 elements of type/class T.
    data = tpie::tpie_new_array<T>(dim0 * dim1);
}

template <class T>  
VarArray2D<T>::VarArray2D(const VarArray2D& other) {
    *this = other;
}

template <class T>  
VarArray2D<T>::~VarArray2D() {
    // Free allocated memory.
	tpie::tpie_delete_array(data, dim[0] * dim[1]);
}

template <class T>  
VarArray2D<T>& VarArray2D<T>::operator=(const VarArray2D& other) {
	if (this == &other) return *this;
	tpie::tpie_delete_array(data, dim[0] * dim[1]);

	this->dim[0] = other.dim[0];
	this->dim[1] = other.dim[1];
	
	//  Allocate memory for dim0 * dim1 elements of type/class T.
	data = tpie::tpie_new_array<T>(dim[0] * dim[1]);
	
	//  Copy objects.
	std::copy(other.data, other.data + dim[0] * dim[1], data);
    return (*this);	
}

template <class T>  
T& VarArray2D<T>::operator()(TPIE_OS_SIZE_T index0, TPIE_OS_SIZE_T index1) {
    assert(index0 < size(0));
    assert(index1 < size(1));
    
    return data[index0 * size(1) + index1];
}

template <class T>  
const T& VarArray2D<T>::operator()(TPIE_OS_SIZE_T index0, TPIE_OS_SIZE_T index1) const {
    assert(index0 < size(0));
    assert(index1 < size(1));
    
    return data[index0 * size(1) + index1];
}

template <class T>  
TPIE_OS_SIZE_T VarArray2D<T>::size() const {
    return dim[0] * dim[1];
}

template <class T>  
TPIE_OS_SIZE_T VarArray2D<T>::size(TPIE_OS_SIZE_T d) const {
    assert(d<2);
    
    return dim[d];
}


//----------------------------------------------------------------------

template <class T>  
VarArray3D<T>::VarArray3D(TPIE_OS_SIZE_T dim0, TPIE_OS_SIZE_T dim1, TPIE_OS_SIZE_T dim2) {
    this->dim[0] = dim0;
    this->dim[1] = dim1;
    this->dim[2] = dim2;
    //  Allocate memory for dim0 * dim1 * dim2 elements of type/class T.
    data = tpie::tpie_new_array<T>(dim0 * dim1 * dim2);
}

template <class T>  
VarArray3D<T>::VarArray3D(const VarArray3D& other) {
    *this = other;
}

template <class T>  
VarArray3D<T>::~VarArray3D() {
    // Free allocated memory.
	tpie_delete_array(data, size());
}

template <class T>  
VarArray3D<T>& VarArray3D<T>::operator=(const VarArray3D& other) {
    if (this == &other) return this;
	
	dim[0] = other.dim[0];
	dim[1] = other.dim[1];
	dim[2] = other.dim[2];

	tpie::tpie_delete_array(data, size());
	//  Allocate memory for dim0 * dim1 * dim2 elements of type/class T.
	data = tpie::tpie_new_array<T>(size());
	
	//  Copy objects.
	std::copy(other.data, other.data + size(), data);
  
	return *this;	
}

template <class T>  
T& VarArray3D<T>::operator()(TPIE_OS_SIZE_T index0, TPIE_OS_SIZE_T index1, TPIE_OS_SIZE_T index2) {
    assert(index0 < size(0));
    assert(index1 < size(1));
    assert(index2 < size(2));
    
    return data[index0 * size(1) * size(2) + index1 * size(2) + index2];
}

template <class T>  
const T& VarArray3D<T>::operator()(TPIE_OS_SIZE_T index0, TPIE_OS_SIZE_T index1, TPIE_OS_SIZE_T index2) const {
    assert(index0 < size(0));
    assert(index1 < size(1));
    assert(index2 < size(2));
    
    return data[index0 * size(1) * size(2) + index1 * size(2) + index2];
}

template <class T>  
TPIE_OS_SIZE_T VarArray3D<T>::size() const {
    
    return dim[0] * dim[1] * dim[2];
}

template <class T>  
TPIE_OS_SIZE_T VarArray3D<T>::size(TPIE_OS_SIZE_T d) const {
    assert(d<3);
    
    return dim[d];
}

//----------------------------------------------------------------------

#endif
