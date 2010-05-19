//	created by Sebastian Reiter
//	s.b.reiter@googlemail.com
//	y09 m11 d17

#ifndef __H__LIB_GRID__DISTRIBUTION_UTIL__
#define __H__LIB_GRID__DISTRIBUTION_UTIL__

#include <vector>
#include <map>
#include "lib_grid/lg_base.h"
#include "grid_distribution.h"
#include "parallel_grid_layout.h"

namespace ug
{

////////////////////////////////////////////////////////////////////////
///	The interface entry holds a local id and the type of the entry.
struct DistributionInterfaceEntry
{
	DistributionInterfaceEntry()	{}
	DistributionInterfaceEntry(int nLocalID, int nType) : localID(nLocalID), type(nType)	{}
	
	int localID : 28;
	int type  	: 4;
};

////////////////////////////////////////////////////////////////////////
///	Holds nodes and interfaces. Used during distribution only.
/**
 * This class is used in the process of grid-distribution to assemble
 * the nodes and interfaces that belong to the different processes.
 * It is however not used during parallel communication.
 */
template <class TNode>
struct DistributionNodeLayout
{
// typedefs
///	the interface entry type
	typedef DistributionInterfaceEntry		InterfaceEntry;
///	an interface consists of a list of local ids.
	typedef std::vector<InterfaceEntry>		Interface;
///	an interface-map is a list of interfaces, each associated with a process id.
	typedef std::map<int, Interface>		InterfaceMap;
///	a list of interface-maps. Required for multilevel / hierarchical approaches.
	typedef std::vector<InterfaceMap>		InterfaceMapVec;

///	the type of the nodes
	typedef TNode	NodeType;
///	a vector that holds nodes.
	typedef std::vector<TNode>				NodeVec;	
	
//	some methods
/*
///	set process id. Use with care to not invalidate other ProcessLayouts.
	inline void set_proc_id(int procID)						{m_procID = procID;}
	
///	returns the process id.
	inline int get_proc_id() const							{return m_procID;}
*/	
///	returns a reference to the vector that holds the nodes.
	inline NodeVec& node_vec()								{return m_vNodes;}
	
///	returns the interface to the given process on the given level.
	/**	if you don't specify a level, level = 0 will be used.*/
	inline Interface& interface(int procID, size_t level = 0)	{return interface_map(level)[procID];}
	
///	returns the interface-map for the given level.
	/**	if you don't specify a level, level = 0 will be used.*/
	inline InterfaceMap& interface_map(size_t level = 0)		{if(level >= m_vInterfaceMaps.size()) m_vInterfaceMaps.resize(level + 1); return m_vInterfaceMaps[level];}
	
///	sets the number of levels.
	/**	Setting the number of levels is optional. Increases performance for #levels > 1.*/
	void set_num_levels(size_t num)							{m_vInterfaceMaps.resize(num);}
	
///	returns the number of levels.
	inline size_t num_levels() const						{return m_vInterfaceMaps.size();}
	
protected:
	//int				m_procID;
	NodeVec			m_vNodes;
	InterfaceMapVec	m_vInterfaceMaps;
};


////////////////////////////////////////////////////////////////////////
//	typedefs
typedef DistributionNodeLayout<VertexBase*>	DistributionVertexLayout;
typedef DistributionNodeLayout<EdgeBase*>	DistributionEdgeLayout;
typedef DistributionNodeLayout<Face*>		DistributionFaceLayout;
typedef DistributionNodeLayout<Volume*>		DistributionVolumeLayout;

////////////////////////////////////////////////////////////////////////
//	CreateDistributionLayouts
///	Creates distribution layouts for vertices, edges, faces and volumes
/**
 * Given a MultiGrid and a SubsetHandler, this method creates distribution
 * layouts for vertices, edges, ...
 * Those layouts can then be used to distribute a grid onto different
 * processes. Please note that those layouts are not used to perform
 * communication later on. Their sole purpose is to help to distribute
 * a grid.
 *
 * For each subset a separate distribution-layout is created. That means 
 * that i.e. vertexLayoutsOut[k] holds the vertex-layout for the k-th subset.
 *
 * If you pass a pointer to a valid selector (which is registered at mg),
 * the selector will be used for internal calculations. The only reason
 * for this parameter is a speed increase.
 * You shouldn't assume anything about the content of pSel after the
 * method finished.
 */
void CreateDistributionLayouts(
						std::vector<DistributionVertexLayout>& vertexLayoutsOut,
						std::vector<DistributionEdgeLayout>& edgeLayoutsOut,
						std::vector<DistributionFaceLayout>& faceLayoutsOut,
						std::vector<DistributionVolumeLayout>& volumeLayoutsOut,
						MultiGrid& mg, SubsetHandler& sh,
						MGSelector* pSel = NULL);

////////////////////////////////////////////////////////////////////////
//	SerializeGridAndDistributionLayouts
/**
 * Writes the elements of a grid, which are referenced by the given
 * gridLayout to a binary stream.
 * You may pass a selector via pSel, which increases performance of this
 * method. After this method finished pSel will contain all elements
 * that have been written to the stream (in the same order as they were
 * written).
 */
void SerializeGridAndDistributionLayouts(
							std::ostream& out, MultiGrid& mg,
							DistributionVertexLayout& vrtLayout,
							DistributionEdgeLayout& edgeLayout,
							DistributionFaceLayout& faceLayout,
							DistributionVolumeLayout& volLayout,
							AInt& aLocalIndVRT, AInt& aLocalIndEDGE,
							AInt& aLocalIndFACE, AInt& aLocalIndVOL,
							MGSelector* pSel = NULL,
							std::vector<int>* pProcessMap = NULL);

////////////////////////////////////////////////////////////////////////
//	SerializeDistributionLayoutInterfaces
template <class TLayout>
void SerializeDistributionLayoutInterfaces(
									std::ostream& out, TLayout& layout,
									std::vector<int>* pProcessMap = NULL);

////////////////////////////////////////////////////////////////////////
//	DeserializeGridAndDistributionLayouts
void DeserializeGridAndDistributionLayouts(
									MultiGrid& mgOut,
									GridLayoutMap& gridLayoutOut,
									std::istream& in);

////////////////////////////////////////////////////////////////////////
//	DeserializeDistributionLayoutInterfaces
/**
 * TLayoutMap has to be compatible with an
 * std::map<int, ParallelELEMENTLayout>, where ELEMENT can be either
 * Vertex, Edge, Face or Volume.
 */
template <class TGeomObj, class TLayoutMap>
void DeserializeDistributionLayoutInterfaces(
									TLayoutMap& layoutMapOut,
									std::vector<TGeomObj*> vGeomObjs,
									std::istream& in);
}//	end of namespace

////////////////////////////////
//	include implementation
#include "distribution_util_impl.hpp"

#endif
