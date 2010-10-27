
#ifndef __H__UG__MARTIN_ALGEBRA__BLOCK_DENSE__
#define __H__UG__MARTIN_ALGEBRA__BLOCK_DENSE__

namespace ug{

template<typename A>
inline double BlockNorm2(const DenseMatrix<A> &mat)
{
	double sum=0;
	for(size_t r=0; r < mat.num_rows(); r++)
		for(size_t c=0; c < mat.num_cols(); c++)
			sum += BlockNorm2(mat(r, c));

	return sum;
}


template<typename A>
inline double BlockNorm2(const DenseVector<A> &v)
{
	double sum=0;
	for(size_t i=0; i < v.size(); i++)
		sum += BlockNorm2(v[i]);
	return sum;
}




//////////////////////////////////////////////////////
// algebra stuff to avoid temporary variables

// vec = mat * vec
// todo: replace add_mult etc. with template expressions
// dest = b*vec
template<typename A, typename B, typename C>
inline void AssignMult(DenseVector<A> &dest, const DenseMatrix<B> &mat, const DenseVector<C> &vec)
{
	UG_ASSERT(mat.num_cols() == vec.size(), "");
	dest.resize(mat.num_rows());
	for(size_t r=0; r < mat.num_rows(); r++)
	{
		AssignMult(dest(r), mat(r, 0), vec(0));
		for(size_t c=1; c < mat.num_cols(); c++)
			AddMult(dest(r), mat(r, c), vec(c));
	}
}

// dest += b*vec
template<typename A, typename B, typename C>
inline void AddMult(DenseVector<A> &dest, const DenseMatrix<B> &mat, const DenseVector<C> &vec)
{
	UG_ASSERT(mat.num_cols() == vec.size(), "");
	dest.resize(mat.num_rows());
	for(size_t r=0; r < mat.num_rows(); r++)
	{
		for(size_t c=0; c < mat.num_cols(); c++)
			AddMult(dest(r), mat(r, c), vec(c));
	}
}


// dest -= b*vec
template<typename A, typename B, typename C>
inline void SubMult(DenseVector<A> &dest, const DenseMatrix<B> &mat, const DenseVector<C> &vec)
{
	UG_ASSERT(mat.num_cols() == vec.size(), "");
	dest.resize(mat.num_rows());
	for(size_t r=0; r < mat.num_rows(); r++)
	{
		for(size_t c=0; c < mat.num_cols(); c++)
			SubMult(dest(r), mat(r, c), vec(c));
	}
}


// mat = alpha * mat

// dest = b*vec
template<typename A, typename B>
inline void AssignMult(DenseMatrix<A> &dest, const double &alpha, const DenseMatrix<B> &mat)
{
	dest.resize(mat.num_rows(), mat.num_cols());
	for(size_t r=0; r < mat.num_rows(); r++)
	{
		for(size_t c=0; c < mat.num_cols(); c++)
			AssignMult(dest(r, c), alpha, mat(r, c));
	}
}

template<typename A, typename B>
inline void AddMult(DenseMatrix<A> &dest, const double &alpha, const DenseMatrix<B> &mat)
{
	dest.resize(mat.num_rows(), mat.num_cols());
	for(size_t r=0; r < mat.num_rows(); r++)
	{
		for(size_t c=0; c < mat.num_cols(); c++)
			AddMult(dest(r, c), alpha, mat(r, c));
	}
}

template<typename A, typename B>
inline void SubMult(DenseMatrix<A> &dest, const double &alpha, const DenseMatrix<B> &mat)
{
	dest.resize(mat.num_rows(), mat.num_cols());
	for(size_t r=0; r < mat.num_rows(); r++)
	{
		for(size_t c=0; c < mat.num_cols(); c++)
			SubMult(dest(r, c), alpha, mat(r, c));
	}
}


// VECTORs

// dest = b*vec
template<typename A, typename B>
inline void AssignMult(DenseVector<A> &dest, const double &b, const DenseVector<B> &vec)
{
	dest.resize(vec.size());
	for(size_t i=0; i<vec.size(); i++)
		AssignMult(dest[i], b, vec[i]);
}

// dest += b*vec
template<typename A, typename B>
inline void AddMult(DenseVector<A> &dest, const double &b, const A &vec)
{
	dest.resize(vec.size());
	for(size_t i=0; i<vec.size(); i++)
		AddMult(dest[i], b, vec[i]);
}

// dest -= b*vec
template<typename A, typename B>
inline void SubMult(DenseVector<A> &dest, const double &b, const DenseVector<B> &vec)
{
	dest.resize(vec.size());
	for(size_t i=0; i<vec.size(); i++)
		SubMult(dest[i], b, vec[i]);
}

// dest = vec*b
template<typename A> inline void AssignMult(A &dest, const A &vec, const double &b)
{
	AssignMult(dest, b, vec);
}
// dest += vec*b
template<typename A> inline void AddMult(A &dest, const A &vec, const double &b)
{
	AddMult(dest, b, vec);
}
// dest -= vec*b
template<typename A> inline void SubMult(A &dest, const A &vec, const double &b)
{
	SubMult(dest, b, vec);
}


template<typename T>
inline void SetSize(DenseMatrix<T> &t, size_t a, size_t b)
{
	t.resize(a, b);
}

//setSize(t, a) for vectors
template<typename T>
inline void SetSize(DenseVector<T> &t, size_t a)
{
	t.resize(a);
}

// getSize
template<typename T>
inline size_t GetSize(DenseVector<T> &t)
{
	return t.size();
}

//getRows
template<typename T>
inline size_t GetRows(const DenseMatrix<T> &t)
{
	return t.num_rows();
}

template<typename T>
inline size_t GetCols(const DenseMatrix<T> &t)
{
	return t.num_cols();
}


template<typename T>
struct block_matrix_traits< DenseMatrix<T> >
{
	enum { ordering = DenseMatrix<T>::ordering };
	enum { is_static = DenseMatrix<T>::is_static};
	enum { static_num_rows = DenseMatrix<T>::static_num_rows};
	enum { static_num_cols = DenseMatrix<T>::static_num_cols};

	// todo: to be implemented
	//typedef DenseMatrixInverse inverse_type;
};

template<typename T>
struct block_vector_traits< DenseVector<T> >
{
	enum { is_static = DenseVector<T>::is_static};
	enum { static_size = DenseVector<T>::static_size};
};


template<typename T1, typename T2>
struct block_multiply_traits<DenseMatrix<T1>, DenseVector<T2> >
{
	typedef DenseVector<T2> ReturnType;
};



template<>
struct block_matrix_traits< DenseMatrix< FixedArray2<double, 1, 1> > >
{
	enum { ordering = DenseMatrix< FixedArray2<double, 1, 1> >::ordering };
	enum { is_static = true};
	enum { static_num_rows = 1};
	enum { static_num_cols = 1};

	typedef double inverse_type;
};

template<>
struct block_matrix_traits< DenseMatrix< FixedArray2<double, 2, 2> > >
{
	enum { ordering = DenseMatrix< FixedArray2<double, 2, 2> >::ordering };
	enum { is_static = true};
	enum { static_num_rows = 2};
	enum { static_num_cols = 2};

	typedef DenseMatrix< FixedArray2<double, 2, 2> > inverse_type;
};

template<>
struct block_matrix_traits< DenseMatrix< FixedArray2<double, 3, 3> > >
{
	enum { ordering = DenseMatrix< FixedArray2<double, 3, 3> >::ordering };
	enum { is_static = true};
	enum { static_num_rows = 3};
	enum { static_num_cols = 3};

	typedef DenseMatrix< FixedArray2<double, 3, 3> > inverse_type;
};


} // namespace ug

#endif
