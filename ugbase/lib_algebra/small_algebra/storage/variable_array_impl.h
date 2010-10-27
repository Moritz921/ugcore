/*
 *  variable_array_impl.h
 *
 *  Created by Martin Rupp on 21.07.10.
 *  Copyright 2010 . All rights reserved.
 *
 */

#ifndef __H__UG__COMMON__VARIABLE_ARRAY_IMPL_H__
#define __H__UG__COMMON__VARIABLE_ARRAY_IMPL_H__

#include "storage.h"
#include "variable_array.h"


namespace ug{

// 'tors
template<typename T>
VariableArray1<T>::VariableArray1()
{
	n = 0;
	values = NULL;
}


template<typename T>
VariableArray1<T>::VariableArray1(size_t n_)
{
	n = 0;
	values = NULL;
	resize(n_);
}

template<typename T>
VariableArray1<T>::VariableArray1(const VariableArray1<T> &other)
{
	n = 0;
	if(values) { delete[] values; values = NULL; }
	resize(other.size());
	for(size_type i=0; i<n; i++)
		values[i] = other[i];
}

template<typename T>
VariableArray1<T>::~VariableArray1()
{
	if(values) { delete[] values; values = NULL; }
	n = 0;
}


// Capacity

template<typename T>
inline size_t
VariableArray1<T>::size() const
{
	return n;
}

template<typename T>
void
VariableArray1<T>::resize(size_t newN)
{
	if(newN == n) return;
	if(values) delete[] values;
	n = newN;
	if(n <= 0)
	{
		values = NULL;
		n = 0;
		return;
	}
	values = new T[n];
	//memset(values, 0, sizeof(T)*n); // ?
}

template<typename T>
inline size_t
VariableArray1<T>::capacity() const
{
	return n;
}


// use stl::vector if you want to use reserve
template<typename T>
inline void
VariableArray1<T>::reserve(size_t newCapacity) const
{
	return;
}


// Element access

template<typename T>
T &
VariableArray1<T>::at(size_t i)
{
	assert(values);
	assert(i>=0 && i<n);
	return values[i];
}

template<typename T>
const T &
VariableArray1<T>::at(size_t i) const
{
	assert(values);
	assert(i>=0 && i<n);
	return values[i];
}

template<typename T>
std::ostream &operator << (std::ostream &out, const VariableArray1<T> &arr)
{
	//out << "VariableArray (n=" << arr.size() << ") [ ";
	for(size_t i=0; i<arr.size(); i++)
		out << arr[i] << " ";
	out << "]";
	return out;
}


////////////////////////////////////////////////////////////////////////////////


template<typename T, eMatrixOrdering T_ordering>
VariableArray2<T, T_ordering>::VariableArray2()
{
	values = NULL;
	rows = 0;
	cols = 0;
}


template<typename T, eMatrixOrdering T_ordering>
VariableArray2<T, T_ordering>::VariableArray2(size_t rows, size_t cols)
{
	values = NULL;
	rows = 0;
	cols = 0;
	resize(rows, cols);
}

template<typename T, eMatrixOrdering T_ordering>
VariableArray2<T, T_ordering>::VariableArray2(const VariableArray2<T, T_ordering> &other)
{
	if(values) { delete[] values; values = NULL; }
	rows = 0;
	cols = 0;
	resize(other.num_rows(), other.num_cols());
	for(size_type i=0; i<rows*cols; i++)
		values[i] = other[i];
}

template<typename T, eMatrixOrdering T_ordering>
VariableArray2<T, T_ordering>::~VariableArray2()
{
	if(values) { delete[] values; values = NULL; }
	rows = cols = 0;
}

// Capacity

template<typename T, eMatrixOrdering T_ordering>
size_t
VariableArray2<T, T_ordering>::num_rows() const
{
	return rows;
}

template<typename T, eMatrixOrdering T_ordering>
size_t
VariableArray2<T, T_ordering>::num_cols() const
{
	return cols;
}


template<typename T, eMatrixOrdering T_ordering>
void
VariableArray2<T, T_ordering>::resize(size_t newRows, size_t newCols)
{
	assert(newRows >= 0 && newCols >= 0);
	if(newRows == rows && newCols == cols) return;
	if(values) delete[] values;
	rows = newRows;
	cols = newCols;
	if(rows == 0 && cols == 0)
	{
		values = NULL;
		return;
	}
	values = new T[rows*cols];
	memset(values, 0, sizeof(T)*rows*cols);
}


template<typename T, eMatrixOrdering T_ordering>
T &
VariableArray2<T, T_ordering>::at(size_t r, size_t c)
{
	assert(r>=0 && r<rows);
	assert(c>=0 && c<cols);
	if(T_ordering==RowMajor)
		return values[c+r*cols];
	else
		return values[r+c*rows];
}

template<typename T, eMatrixOrdering T_ordering>
const T &
VariableArray2<T, T_ordering>::at(size_t r, size_t c) const
{
	assert(r>=0 && r<rows);
	assert(c>=0 && c<cols);
	if(T_ordering==RowMajor)
		return values[c+r*cols];
	else
		return values[r+c*rows];
}

template<typename T, eMatrixOrdering T_ordering>
std::ostream &operator << (std::ostream &out, const VariableArray2<T, T_ordering> &arr)
{
	out << "[ ";
	//out << "VariableArray2 (" << arr.num_rows() << "x" << arr.num_cols() << "), " << ((T_ordering == ColMajor) ? "ColMajor" : "RowMajor") << endl;
	typedef size_t size_type;
	for(size_type r=0; r<arr.num_rows(); r++)
	{
		for(size_type c=0; c<arr.num_cols(); c++)
			out << arr(r, c) << " ";
		if(r != arr.num_rows()-1) out << "| ";
	}
	out << "]";
	return out;
}

}
#endif // __H__UG__COMMON__VARIABLE_ARRAY_IMPL_H__
