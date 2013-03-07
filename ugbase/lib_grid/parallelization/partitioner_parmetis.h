// created by Sebastian Reiter
// s.b.reiter@gmail.com
// Feb 25, 2013 (d,m,y)

#ifndef __H__UG__partitioner_parmetis__
#define __H__UG__partitioner_parmetis__

#include "parallel_grid_layout.h"
#include "load_balancer.h"
#include "pcl/pcl_interface_communicator.h"
#include "util/parallel_dual_graph.h"

extern "C" {
	#include "metis.h"
	#include "parmetis.h"
}

namespace ug{

template <int dim>
class Partitioner_Parmetis : public IPartitioner<dim>{
	public:
		typedef IPartitioner<dim> base_class;
		typedef typename base_class::elem_t	elem_t;

		Partitioner_Parmetis();
		virtual ~Partitioner_Parmetis();

		virtual void set_grid(MultiGrid* mg, Attachment<MathVector<dim> > aPos);
		virtual void set_process_hierarchy(SPProcessHierarchy procHierarchy);
		virtual void set_balance_weights(SmartPtr<BalanceWeights<dim> > balanceWeights);
		virtual void set_connection_weights(SmartPtr<ConnectionWeights<dim> > conWeights);

		virtual bool supports_balance_weights() const;
		virtual bool supports_connection_weights() const;

		virtual void partition(size_t baseLvl, size_t elementThreshold);

		virtual SubsetHandler& get_partitions();
		virtual const std::vector<int>* get_process_map() const;

	private:
	///	fills m_aNumChildren with child-counts from levels baseLvl to topLvl.
	/**	Elements in topLvl have child-count 0.*/
		void accumulate_child_counts(int baseLvl, int topLvl, AInt aInt);

	/**	make sure that m_numChildren contains a valid number for all elements
	 * on the given level!*/
		void partition_level_metis(int lvl, int numTargetProcs);

	/**	make sure that m_numChildren contains a valid number for all elements
	 * on the given level!
	 * The given procCom should contain all processes which are potentially
	 * involved on the given level (before or after distribution).*/
		void partition_level_parmetis(int lvl, int numTargetProcs,
									  const pcl::ProcessCommunicator& procComAll,
									  ParallelDualGraph<elem_t, idx_t>& pdg);

		typedef SmartPtr<BalanceWeights<dim> >		SPBalanceWeights;
		typedef SmartPtr<ConnectionWeights<dim> >	SPConnectionWeights;
		typedef typename GridLayoutMap::Types<elem_t>::Layout::LevelLayout	layout_t;

		MultiGrid* m_mg;
		SubsetHandler m_sh;
		AInt m_aNumChildren;
		Grid::AttachmentAccessor<elem_t, AInt>	m_aaNumChildren;
		SPBalanceWeights	m_balanceWeights;
		SPConnectionWeights	m_connectionWeights;
		SPProcessHierarchy	m_processHierarchy;
		pcl::InterfaceCommunicator<layout_t>	m_intfcCom;
};


} // end of namespace

#endif
