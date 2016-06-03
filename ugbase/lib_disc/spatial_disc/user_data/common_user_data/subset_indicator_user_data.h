/*
 * Copyright (c) 2015:  G-CSC, Goethe University Frankfurt
 * Author: Dmitry Logashenko
 * 
 * This file is part of UG4.
 * 
 * UG4 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License version 3 (as published by the
 * Free Software Foundation) with the following additional attribution
 * requirements (according to LGPL/GPL v3 §7):
 * 
 * (1) The following notice must be displayed in the Appropriate Legal Notices
 * of covered and combined works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (2) The following notice must be displayed at a prominent place in the
 * terminal output of covered works: "Based on UG4 (www.ug4.org/license)".
 * 
 * (3) The following bibliography is recommended for citation and must be
 * preserved in all covered files:
 * "Reiter, S., Vogel, A., Heppner, I., Rupp, M., and Wittum, G. A massively
 *   parallel geometric multigrid solver on hierarchically distributed grids.
 *   Computing and visualization in science 16, 4 (2013), 151-164"
 * "Vogel, A., Reiter, S., Rupp, M., Nägel, A., and Wittum, G. UG4 -- a novel
 *   flexible software system for simulating pde based models on high performance
 *   computers. Computing and visualization in science 16, 4 (2013), 165-179"
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 */

/*
 * User data of a subset indicator (1 in the subset, 0 everywhere else)
 */
#ifndef __H__UG__LIB_DISC__SPATIAL_DISC__USER_DATA__SSIND_USER_DATA__
#define __H__UG__LIB_DISC__SPATIAL_DISC__USER_DATA__SSIND_USER_DATA__

#include <vector>

// ug4 headers
#include "common/common.h"
#include "common/math/ugmath.h"
#include "lib_disc/spatial_disc/user_data/std_glob_pos_data.h"

namespace ug {

/// User data of a subset indicator (1 in the subset, 0 everywhere else)
/**
 * User data based class for the indicator function.
 * The subset names specified in the constructor define a subdomain \f$ S \f$
 * of the whole domain \f$ \Omega \f$. This user data class computes the
 * indicator function that is \f$ 1 \f$ in \f$ S \f$ and zero in
 * \f$ \Omega \setminus S \f$.
 */
template <typename TDomain>
class SubsetIndicatorUserData
	: public StdUserData<SubsetIndicatorUserData<TDomain>, number, TDomain::dim, void, UserData<number, TDomain::dim, void> >
{
public:
///	Type of domain
	typedef TDomain domain_type;
	
///	World dimension
	static const int dim = domain_type::dim;
	
/// subset handler type
	typedef typename domain_type::subset_handler_type subset_handler_type;

private:
	/// subset group representing the specified subdomain
	SubsetGroup m_ssGrp;
	
public:

///	Constructor
	SubsetIndicatorUserData
	(
		ConstSmartPtr<domain_type> domain, ///< domain of the problem
		const char * ss_names ///< subset names
	)
	: m_ssGrp (domain->subset_handler ())
	{
	// Parse the subset names:
		std::vector<std::string> vssNames;
		try
		{
			TokenizeString (ss_names, vssNames);
			for (size_t k = 0; k < vssNames.size (); k++)
				RemoveWhitespaceFromString (vssNames [k]);
			m_ssGrp.clear ();
			m_ssGrp.add (vssNames);
		} UG_CATCH_THROW ("SubsetIndicatorUserData: Failed to parse subset names.");
	}

///	Indicator functions are discontinuous
	virtual bool continuous () const {return false;}

///	Returns true to get the grid element in the evaluation routine
	virtual bool requires_grid_fct () const {return true;}

///	Evaluator
	template <int refDim>
	inline void evaluate
	(
		number vValue [],
		const MathVector<dim> vGlobIP [],
		number time,
		int si,
		GridObject * elem,
		const MathVector<dim> vCornerCoords [],
		const MathVector<refDim> vLocIP [],
		const size_t nip,
		LocalVector * u,
		const MathMatrix<refDim, dim> * vJT = NULL
	) const
	{
	//	Get the subset index of the element
		int elem_si = m_ssGrp.subset_handler()->get_subset_index (elem);
	//	Check if the element is in one of the specified subsets:
		number indicator = (m_ssGrp.contains (elem_si))? 1 : 0;
	//	Return the indicator:
		for (size_t i = 0; i < nip; i++)
			vValue [i] = indicator;
	};
	
///	This function should not be used
	void operator()
	(
		number & vValue,
		const MathVector<dim> & globIP,
		number time,
		int si
	)
	const
	{
		UG_THROW("SubsetIndicatorUserData: Element required for evaluation, but not passed. Cannot evaluate.");
	}

///	This function should not be used
	void operator()
	(
		number vValue [],
		const MathVector<dim> vGlobIP [],
		number time,
		int si,
		const size_t nip
	) const
	{
		UG_THROW("SubsetIndicatorUserData: Element required for evaluation, but not passed. Cannot evaluate.");
	}
};

} // end namespace ug

#endif // __H__UG__LIB_DISC__SPATIAL_DISC__USER_DATA__SSIND_USER_DATA__

/* End of File */
