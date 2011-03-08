#include "MPPMarkingFactory.hpp"
#include <cstdlib>

namespace VerifyTAPN {

	id_type MPPMarkingFactory::nextId = 0;

	SymbolicMarking *MPPMarkingFactory::InitialMarking(const DiscretePart &dp) const {
		MPPMarking *marking = new MPPMarking(dp);
		marking->initZero();
		marking->id = nextId++;
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

