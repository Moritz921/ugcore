//	created by Sebastian Reiter
//	s.b.reiter@googlemail.com
//	y09 m09 d17

#ifndef __H__LIB_GRID__TETRAHEDRALIZATION__
#define __H__LIB_GRID__TETRAHEDRALIZATION__

#include "lib_grid/lg_base.h"

namespace ug
{

/// \addtogroup lib_grid_algorithms_grid_generation
///	@{

////////////////////////////////////////////////////////////////////////
///	fills a closed surface-grid with tetrahedrons.
/**	You may specify a quality parameter. If this parameter is <= 0, no
 *	inner vertices will be created. The default value for quality is 5.
 *	The quality resembles the minimal valid dihedral angle.
 *	The algorithm should always terminate for this quality. If you choose
 *	a lower quality parameter (careful with quality < 1), the algotithm may
 *	not terminate. The lower the quality parameter (but > 0), the
 *	better the tetrahedron quality.
 *
 *	Using tetgen by Hang Si.
 *	\{
 */
bool Tetrahedralize(Grid& grid, number quality = 5,
					bool preserveBnds = false,
					bool preserveAll = false,
					APosition& aPos = aPosition);

bool Tetrahedralize(Grid& grid, SubsetHandler& sh,
					number quality = 5,
					bool preserveBnds = false,
					bool preserveAll = false,
					APosition& aPos = aPosition);
///	\}

///	If tetrahedrons are already present, this method refines them based on the given volume constraints.
/**	A negative volume constraint implies no constraint for that element.*/
bool Retetrahedralize(Grid& grid, SubsetHandler& sh,
					ANumber& aVolumeConstraint,
					number quality = 5,
					bool preserveBnds = false,
					bool preserveAll = false,
					APosition& aPos = aPosition);
/**@}*/ // end of doxygen defgroup command

}//	end of namespace

#endif
