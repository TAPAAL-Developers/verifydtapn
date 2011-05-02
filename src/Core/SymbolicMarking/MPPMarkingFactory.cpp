#include "MPPMarkingFactory.hpp"

namespace VerifyTAPN {

	id_type MPPMarkingFactory::nextId = 1;

	MPPMarkingFactory::MPPMarkingFactory(const boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn, int clocks) : clocks(clocks) {
		MPPMarking::tapn = tapn;
	};

	SymbolicMarking *MPPMarkingFactory::InitialMarking(const std::vector<int>& tokenPlacement) const {
#ifdef DBM_NORESIZE
		MPPMarking *marking = new MPPMarking(DiscretePart(tokenPlacement), clocks);
#else
		MPPMarking *marking = new MPPMarking(DiscretePart(tokenPlacement));
#endif
		marking->InitZero();
		marking->id = 0;
		return marking;
	}

	SymbolicMarking *MPPMarkingFactory::Clone(const SymbolicMarking &marking) const {
		MPPMarking *clone = new MPPMarking(static_cast<const MPPMarking&>(marking));
		clone->id = nextId++;
		return clone;
	}

	SymbolicMarking *MPPMarkingFactory::Convert(StoredMarking *marking) const {
		return static_cast<MPPMarking*>(marking);
	}

	StoredMarking *MPPMarkingFactory::Convert(SymbolicMarking *marking) const {
		return static_cast<MPPMarking*>(marking);
	}
}

