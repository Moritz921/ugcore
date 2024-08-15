/*
 * Copyright (c) 2010-2015:  G-CSC, Goethe University Frankfurt
 * Authors: Sebastian Reiter, Andreas Vogel, Jan Friebertshäuser
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

#ifndef __H__UG__LIB_GRID__TOOLS__CREATOR_GRID_UTIL__
#define __H__UG__LIB_GRID__TOOLS__CREATOR_GRID_UTIL__

// other lib_discretization headers


namespace ug{
////////////////////////////////////////////////////////////////////////
/**
 * To Create GridObject directly under UG4-C++ 
 * instead of ProMesh 
 */
template<typename TPosition>
Vertex* CreateVertex(Grid& grid,const TPosition& pos,Grid::VertexAttachmentAccessor<Attachment<TPosition> >& aaPos )
{
    RegularVertex* vrt = *grid.create<RegularVertex>();

	if(vrt){
		aaPos[vrt] = pos;
    }

    return vrt;
}



Edge* CreateEdge(Grid& grid, Vertex* vrts[], const size_t, numVrts){
    //uniquify vrts by hash_value: no
    //The vrts can't be simply uniquified by hash_values since it will change the orientation of vrts. 
    // Hence,it should be generated by aaPos before using this function.
    RegularEdge* e = NULL;

     switch(numVrts){
		case 2:{//	create edge
				if(!grid.get_edge(vrts[0], vrts[1]))
					e = *grid.create<ug::RegularEdge>(ug::EdgeDescriptor(vrts[0], vrts[1]));
				else{
					UG_LOG("Can't create edge: RegularEdge already exists.\n");
				}
			}break;

		default:
			UG_LOG("Can't create edge: Bad number of vertices. 2 are required.\n");
			break;
	}
    return e;  

}

Edge* CreateEdge(Grid& grid, Vertex* vrt0, Vertex* vrt1)
{
    RegularEdge* e = NULL;

    uint32 hash0=vrt0->get_hash_value();
    uint32 hash1=vrt1->get_hash_value();
    UG_ASSERT(hash0!=hash1, "Can't create edge: they are identical vertices."); 
        
    Vertex* vrts[2];
    vrts[0]=vrt0;
    vrts[1]=vrt1;
    //	create edge
   return CreateEdge(grid, vrts, 2);
}

/*Edge* CreateEdge(Grid& grid, std::vector<Vertex*> vrts){
    size_t numVrts = vrts.num_vertices();
    // vector -> array    
    for(size_t i=0; i<numVrts; ++i){       
    }
    //return CreateEdge(grid, avrts, numVrts);
}*/

Face* CreateFace(Grid& grid, Vertex* vrts[], const size_t numVrts){
    //if(numVrts < 3 || numVrts > 4){
	UG_ASSERT(numVrts>2 || numVrts<5,"Bad number of vertices! Can't create a face element from " << numVrts << " vertices.");
		//return NULL;
	//}

    FaceDescriptor fd(numVrts);
	for(size_t i = 0; i < numVrts; ++i)
		fd.set_vertex(i, vrts[i]);

	if(grid.get_face(fd)){
		UG_LOG("A face connecting the given vertices already exists. Won't create a new one!");
		return NULL;
	}

	Face* f = NULL;
	switch(numVrts){
		case 3:{//	create triangle
			f = *grid.create<Triangle>(TriangleDescriptor(vrts[0], vrts[1], vrts[2]));
		}break;

		case 4:{//	create quadrilateral
			f = *grid.create<Quadrilateral>(QuadrilateralDescriptor(vrts[0], vrts[1],
																			vrts[2], vrts[3]));
		}break;

		default:
			UG_LOG("Can't create face: Bad number of vertices. 3 or 4 are supported.\n");
			break;
	}

    return f;
}

/*Face* CreateFace(Grid& grid, std::vector<Vertex*> vrts){
    size_t numVrts = vrts.num_vertices();
    // vector -> array    
    for(size_t i=0; i<numVrts; ++i){           
    }
    //return CreateFace(grid, avrts, numVrts);
}*/

Volume* CreateVolume(Grid& grid, Vertex* vrts, const size_t numVrts){
//if(numVrts < 4 || numVrts > 8){
	UG_ASSERT(numVrts>3 || numVrts<9,"Bad number of vertices! Can't create a volume element from " << numVrts << " vertices.");

    VolumeDescriptor vd(numVrts);
	for(size_t i = 0; i < numVrts; ++i)
		vd.set_vertex(i, vrts[i]);

	if(grid.get_volume(vd)){
		UG_LOG("A volume connecting the given vertices already exists. Won't create a new one!");
		return NULL;
	}

	Volume* v = NULL;
	switch(numVrts){
		case 4:{//	create tetrahedron
			v = *grid.create<Tetrahedron>(TetrahedronDescriptor(vrts[0], vrts[1],
																vrts[2], vrts[3]));
		}break;

		case 5:{//	create pyramid
			v = *grid.create<Pyramid>(PyramidDescriptor(vrts[0], vrts[1],
														vrts[2], vrts[3], vrts[4]));
		}break;

		case 6:{//	create prism
			v = *grid.create<Prism>(PrismDescriptor(vrts[0], vrts[1], vrts[2],
													vrts[3], vrts[4], vrts[5]));
		}break;

		case 8:{//	create hexahedron
			v = *grid.create<Hexahedron>(HexahedronDescriptor(vrts[0], vrts[1], vrts[2], vrts[3],
															  vrts[4], vrts[5], vrts[6], vrts[7]));
		}break;

		default:
			UG_LOG("Can't create volume: Bad number of vertices. 4, 5, 6, and 8 are supported.\n");
			break;
	}

    return v;
}

/*Volume* CreateVolume(Grid& grid, std::vector<Vertex*> vrts){
    size_t numVrts = vrts.num_vertices();
    // vector -> array    
    for(size_t i=0; i<numVrts; ++i){           
    }
    //return CreateVolume(grid, avrts, numVrts);
}*/

/*
template<typename TElem, typename TPosition> //for edges, faces, Volumes. constexpr(TElem:HAS_SIDES)
TElem* CreateGridObject(Grid& grid, 
int vrt_indices[], 
const size_t numVrts, Grid::VertexAttachmentAccessor<Attachment<TPosition> >& aaPOS)
*/

//pos: 0: upLeft,1:lowLeft, 2: lowRight, 3. upRight
template<typename TPosition>
void CreatePlane(Grid& grid, 
                 const TPosition& upLeft,
                 const TPosition& upRight,
                 const TPosition& lowLeft,
                 const TPosition& lowRight,
                 Grid::VertexAttachmentAccessor<Attachment<TPosition> >& aaPos
                 bool fill)
{   
    Vertex* vrts[4];
	for(size_t i = 0; i < 4; ++i)
		vrts[i] = *grid.create<RegularVertex>();

	aaPos[vrts[0]] = upLeft;
	aaPos[vrts[1]] = lowLeft;
	aaPos[vrts[2]] = lowRight;
	aaPos[vrts[3]] = upRight;

    if(fill){
        CreateFace(grid, vrts, 4);
    }else{
        for(size_t i = 0; i < 4; ++i){
			int i0 = i;
			int i1 = (i + 1) % 4;
			grid.create<RegularEdge>(EdgeDescriptor(vrts[i0], vrts[i1]));
		}
    }
}


}

#endif /* __H__UG__LIB_GRID__TOOLS__CREATOR_GRID_UTIL__*/