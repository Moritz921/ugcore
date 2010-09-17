/*
 * disc_helper.h
 *
 *  Created on: 06.09.2010
 *      Author: andreasvogel
 */

#ifndef __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__DISC_HELPER__DISC_HELPER__
#define __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__DISC_HELPER__DISC_HELPER__

#include "./finite_volume_output.h"
#include "./finite_volume_geometry.h"

/*
// Singleton, holding a Finite Volume Geometry
class FVGeometryProvider {

		// private constructor
		FVGeometryProvider();

		// disallow copy and assignment (intentionally left unimplemented)
		FVGeometryProvider(const FVGeometryProvider&);
		FVGeometryProvider& operator=(const FVGeometryProvider&);

		// private destructor
		~FVGeometryProvider(){};

		// geometry provider
		template <typename TElem, int TWorldDim>
		inline static FV1Geometry<TElem, TWorldDim>& inst()
		{
			static FV1Geometry<TElem, TWorldDim> myInst;
			return myInst;
		};

	public:
		// get the local shape function set for a given reference element and id
		template <typename TElem, int TWorldDim>
		inline static FV1Geometry<TElem, TWorldDim>& get_geom()
		{
			return inst<TRefElem, TWorldDim>();
		}
};

*/
#endif /* __H__LIB_DISCRETIZATION__SPACIAL_DISCRETIZATION__DISC_HELPER__DISC_HELPER__ */
