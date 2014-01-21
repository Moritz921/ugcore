/*
 * truncated_monotone_transfer.h
 *
 *  Created on: 20.01.2014
 *      Author: raphaelprohl
 */

#ifndef __H__UG__LIB_DISC__OPERATOR__NON_LINEAR_OPERATOR__TRUNCATED_MONOTONE_MG__TRUNCATED_MONOTONE_TRANSFER_H_
#define __H__UG__LIB_DISC__OPERATOR__NON_LINEAR_OPERATOR__TRUNCATED_MONOTONE_MG__TRUNCATED_MONOTONE_TRANSFER_H_

#include "lib_disc/operator/linear_operator/transfer_interface.h"

namespace ug{

template <typename TDomain, typename TAlgebra>
class TruncatedMonotoneTransfer: public ITransferOperator<TDomain, TAlgebra>
{
	public:
	/// This type
		typedef TruncatedMonotoneTransfer<TDomain, TAlgebra> this_type;

	/// base type
		typedef ITransferOperator<TDomain, TAlgebra> base_type;

	///	Type of Domain
		typedef TDomain domain_type;

	///	Type of algebra
		typedef TAlgebra algebra_type;

	///	Type of Vector
		typedef typename TAlgebra::vector_type vector_type;

	///	Type of Vector
		typedef typename TAlgebra::matrix_type matrix_type;

	/// world dimension
		static const int dim = TDomain::dim;

	public:
	///	Constructor
		TruncatedMonotoneTransfer():
			ITransferOperator<TDomain, TAlgebra>(), m_bInit (false)
		{};

	public:
		//////////////////////////////
		//	INTERFACE METHODS
		//////////////////////////////

	/// Set Levels for Prolongation coarse -> fine
		void set_levels(GridLevel coarseLevel, GridLevel fineLevel);

	public:
	///	initialize the operator
		void init();

	/// Prolongates vector, i.e. moves data from coarse to fine level
		void prolongate(vector_type& uFine, const vector_type& uCoarse);

	/// Restricts vector, i.e. moves data from fine to coarse level
		void do_restrict(vector_type& uCoarse, const vector_type& uFine);

	///	Clone
		SmartPtr<ITransferOperator<TDomain, TAlgebra> > clone();

	private:
	///	init flag
		bool m_bInit;

	///	list of post processes
		using base_type::m_vConstraint;

	///	coarse and fine Gridlevel
		GridLevel m_coarseLevel;
		GridLevel m_fineLevel;

};

} // end namespace ug

// include implementation
#include "truncated_monotone_transfer_impl.h"

#endif /* __H__UG__LIB_DISC__OPERATOR__NON_LINEAR_OPERATOR__TRUNCATED_MONOTONE_MG__TRUNCATED_MONOTONE_TRANSFER_H_ */